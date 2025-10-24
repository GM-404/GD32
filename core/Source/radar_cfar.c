#include "radar_cfar.h"
#include <stdio.h>
#include <stdlib.h> // For malloc, free, qsort
#include <math.h>   // For sqrt, fabs
#include <stdbool.h>
#include <float.h> // For DBL_MAX, DBL_MIN
#include "private.h" // 包含雷达数据结构体定义


CfarParams cfar_params = {
                        .guard_cells_range = CONFIG_CFAR_NUM_GUARD_RANGE,       // 距离维度保护单元 (单侧)
                        .guard_cells_doppler = CONFIG_CFAR_NUM_GUARD_VEL,     // 多普勒维度保护单元 (单侧)
                        .training_cells_range = CONFIG_CFAR_NUM_TRAIN_RANGE,    // 距离维度训练单元 (单侧)
                        .training_cells_doppler = CONFIG_CFAR_NUM_TRAIN_RANGE,  // 多普勒维度训练单元 (单侧)
                        .threshold_factor = CONFIG_CFAR_TH_AMP,     // 阈值因子 (需要根据实际数据调整)
                        .cfar_strategy = CONFIG_CFAR_STRATEGY,            // 0: CA-CFAR (平均), 1: GO-CFAR (最大值)
                        .os_k_rank = CONFIG_CFAR_OS_K
                    };
                    
// 比较函数，用于qsort排序double数组
static int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double*)a;
    double arg2 = *(const double*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

int perform_cfar_detection(const RadarFFT2DOutput input_fft_2d_data,
                        RadarDetectionMap output_detection_map,
                           const CfarParams *params) {
    
    // 检查参数合法性
    if (params->guard_cells_range < 0 || params->guard_cells_doppler < 0 ||
        params->training_cells_range < 0 || params->training_cells_doppler < 0 ||
        params->threshold_factor <= 0) {
        fprintf(stderr, "Error: Invalid CFAR parameters (guard/training cells must be >= 0, threshold factor > 0).\n");
        return -1;
    }
    if (params->cfar_strategy == 3 && params->os_k_rank <= 0) {
        fprintf(stderr, "Error: For OS-CFAR, os_k_rank must be greater than 0.\n");
        return -1;
    }

    // 将2D FFT的复数结果转换为幅度平方 (能量)
    // 使用动态分配，确保不会栈溢出
    double (*power_map)[RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS] = 
        (double (*)[RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS])
        malloc(sizeof(double) * RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS);

    if (power_map == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for CFAR power map.\n");
        return -1;
    }

    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int r_idx = 0; r_idx < RADAR_CHIRP_POINTS; ++r_idx) {
            for (int d_idx = 0; d_idx < RADAR_CHIRP_COUNT; ++d_idx) {
                double real = input_fft_2d_data[ant][d_idx][r_idx][0];
                double imag = input_fft_2d_data[ant][d_idx][r_idx][1];
                power_map[ant][d_idx][r_idx] = real * real + imag * imag;
                output_detection_map[ant][d_idx][r_idx] = 0; // 初始化为未检测
            }
        }
    }

    // 定义CFAR窗口参数
    int G_r = params->guard_cells_range;
    int G_d = params->guard_cells_doppler;
    int T_r = params->training_cells_range;
    int T_d = params->training_cells_doppler;
    double alpha = params->threshold_factor;

    // 预估最大参考单元数量，分配临时数组
    // 一个完整方形窗口的最大训练单元数 (2*Tr + 2*Gr + 1) * (2*Td + 2*Gd + 1) - (2*Gr + 1)*(2*Gd + 1)
    // 对于十字形，最大训练单元数
    // 距离方向上的训练单元总数: 2 * T_r * (2 * G_d + 1)  (不包括中心的 (2*G_r+1) * (2*G_d+1) 部分)
    // 多普勒方向上的训练单元总数: 2 * T_d * (2 * G_r + 1) (不包括中心的 (2*G_r+1) * (2*G_d+1) 部分)
    // 交叉部分：(2*T_r+1)*(2*G_d+1) + (2*T_d+1)*(2*G_r+1) - (2*G_r+1)*(2*G_d+1)
    // 纯粹的十字形训练区数量:
    int max_training_cells = 2 * T_r * (2 * G_d + 1) + 2 * T_d * (2 * G_r + 1);
    if (max_training_cells == 0) max_training_cells = 1; // 至少分配一个，避免malloc(0)行为不确定

    double *training_cell_powers = (double*)malloc(sizeof(double) * max_training_cells);
    if (training_cell_powers == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for training cell powers.\n");
        free(power_map);
        return -1;
    }

    // 遍历每个单元格作为CUT
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int r_cut = 0; r_cut < RADAR_CHIRP_POINTS; ++r_cut) {
            for (int d_cut = 0; d_cut < RADAR_CHIRP_COUNT; ++d_cut) {
                
                int current_training_cell_count = 0;
                
                // === 通用训练单元收集逻辑 (适用于所有十字形策略，但后续处理不同) ===
                // 为了避免重复代码，先收集所有符合十字形保护区外的训练单元
                // 对于CrossMaxMean, CrossMinMean, 需要独立计算四个分支的均值，
                // 所以这里将为每个分支维护一个求和和计数
                
                // 存储四个分支的功率和和计数
                double branch_sums[4] = {0.0, 0.0, 0.0, 0.0}; // 0:上, 1:下, 2:左, 3:右
                int branch_counts[4] = {0, 0, 0, 0};

                // 遍历以CUT为中心的整个窗口，包括保护单元和训练单元
                for (int r_offset = -(G_r + T_r); r_offset <= (G_r + T_r); ++r_offset) {
                    for (int d_offset = -(G_d + T_d); d_offset <= (G_d + T_d); ++d_offset) {
                        
                        // 跳过CUT自身
                        if (r_offset == 0 && d_offset == 0) continue;

                        int r_cell = r_cut + r_offset;
                        int d_cell = d_cut + d_offset;

                        // 检查边界
                        if (r_cell >= 0 && r_cell < RADAR_CHIRP_POINTS &&
                            d_cell >= 0 && d_cell < RADAR_CHIRP_COUNT) {
                            
                            // 确定是否在保护区内
                            bool in_guard_region_range = (abs(r_offset) <= G_r);
                            bool in_guard_region_doppler = (abs(d_offset) <= G_d);

                            // 如果在保护区内，跳过 (不用于噪声估计)
                            if (in_guard_region_range && in_guard_region_doppler) {
                                continue;
                            }

                            // 确定是否是训练单元，并归类到相应的分支
                            // 上分支 (d_offset < -(G_d)) 且在范围保护区内
                            if (d_offset < -(G_d) && d_offset >= -(G_d + T_d) && in_guard_region_range) {
                                branch_sums[0] += power_map[ant][d_cell][r_cell];
                                branch_counts[0]++;
                                if (params->cfar_strategy == 0 || params->cfar_strategy == 3) { // CA-CFAR 或 OS-CFAR需要所有训练单元
                                    if (current_training_cell_count < max_training_cells) {
                                        training_cell_powers[current_training_cell_count++] = power_map[ant][d_cell][r_cell];
                                    }
                                }
                            }
                            // 下分支 (d_offset > G_d) 且在范围保护区内
                            else if (d_offset > G_d && d_offset <= (G_d + T_d) && in_guard_region_range) {
                                branch_sums[1] += power_map[ant][d_cell][r_cell];
                                branch_counts[1]++;
                                if (params->cfar_strategy == 0 || params->cfar_strategy == 3) {
                                    if (current_training_cell_count < max_training_cells) {
                                        training_cell_powers[current_training_cell_count++] = power_map[ant][d_cell][r_cell];
                                    }
                                }
                            }
                            // 左分支 (r_offset < -(G_r)) 且在多普勒保护区内
                            else if (r_offset < -(G_r) && r_offset >= -(G_r + T_r) && in_guard_region_doppler) {
                                branch_sums[2] += power_map[ant][d_cell][r_cell];
                                branch_counts[2]++;
                                if (params->cfar_strategy == 0 || params->cfar_strategy == 3) {
                                    if (current_training_cell_count < max_training_cells) {
                                        training_cell_powers[current_training_cell_count++] = power_map[ant][d_cell][r_cell];
                                    }
                                }
                            }
                            // 右分支 (r_offset > G_r) 且在多普勒保护区内
                            else if (r_offset > G_r && r_offset <= (G_r + T_r) && in_guard_region_doppler) {
                                branch_sums[3] += power_map[ant][d_cell][r_cell];
                                branch_counts[3]++;
                                if (params->cfar_strategy == 0 || params->cfar_strategy == 3) {
                                    if (current_training_cell_count < max_training_cells) {
                                        training_cell_powers[current_training_cell_count++] = power_map[ant][d_cell][r_cell];
                                    }
                                }
                            }
                            // 警告：如果走到这里，说明有一个训练单元既不在任何分支条件中，又不在保护区内。
                            // 这可能发生在角区域，或者训练单元定义不严格的CFAR。
                        }
                    }
                }
                
                // 根据CFAR策略计算参考噪声
                double noise_estimate = 0.0;

                if (params->cfar_strategy == 0) { // CA-CFAR (CrossMean)
                    if (current_training_cell_count == 0) {
                        output_detection_map[ant][d_cut][r_cut] = 0;
                        continue; 
                    }
                    double sum_noise_power = 0.0;
                    for (int i = 0; i < current_training_cell_count; ++i) {
                        sum_noise_power += training_cell_powers[i];
                    }
                    noise_estimate = sum_noise_power / current_training_cell_count;
                } else if (params->cfar_strategy == 1 || params->cfar_strategy == 2) { // GO-CFAR (CrossMaxMean) 或 SO-CFAR (CrossMinMean)
                    // 计算四个分支的平均功率
                    double branch_means[4];
                    int valid_branches = 0;
                    for(int i = 0; i < 4; ++i) {
                        if (branch_counts[i] > 0) {
                            branch_means[i] = branch_sums[i] / branch_counts[i];
                            valid_branches++;
                        } else {
                            // 无效分支，根据策略设置为极大或极小值，使其不影响最终 Max/Min 结果
                            branch_means[i] = (params->cfar_strategy == 1) ? DBL_MIN : DBL_MAX; 
                        }
                    }

                    if (valid_branches == 0) { // 如果所有分支都没有有效训练单元
                        output_detection_map[ant][d_cut][r_cut] = 0;
                        continue;
                    }

                    if (params->cfar_strategy == 1) { // CrossMaxMean
                        noise_estimate = DBL_MIN;
                        for(int i = 0; i < 4; ++i) {
                            if (branch_counts[i] > 0 && branch_means[i] > noise_estimate) {
                                noise_estimate = branch_means[i];
                            }
                        }
                    } else { // CrossMinMean
                        noise_estimate = DBL_MAX;
                        for(int i = 0; i < 4; ++i) {
                            if (branch_counts[i] > 0 && branch_means[i] < noise_estimate) {
                                noise_estimate = branch_means[i];
                            }
                        }
                    }

                } else if (params->cfar_strategy == 3) { // OS-CFAR (CrossOS)
                    if (current_training_cell_count == 0 || params->os_k_rank > current_training_cell_count) {
                        output_detection_map[ant][d_cut][r_cut] = 0;
                        continue; 
                    }
                    qsort(training_cell_powers, current_training_cell_count, sizeof(double), compare_doubles);
                    // OS-CFAR取排序后第 K 大的值 (索引为 current_training_cell_count - K)
                    noise_estimate = training_cell_powers[current_training_cell_count - params->os_k_rank];
                } else {
                    fprintf(stderr, "Warning: Unknown CFAR strategy (%d). Using default CA-CFAR.\n", params->cfar_strategy);
                    if (current_training_cell_count == 0) {
                        output_detection_map[ant][d_cut][r_cut] = 0;
                        continue; 
                    }
                    double sum_noise_power = 0.0;
                    for (int i = 0; i < current_training_cell_count; ++i) {
                        sum_noise_power += training_cell_powers[i];
                    }
                    noise_estimate = sum_noise_power / current_training_cell_count;
                }

                // 计算阈值
                double threshold = alpha * noise_estimate;

                // 待检测单元的能量
                double cut_power = power_map[ant][d_cut][r_cut];

                // 检测判断
                if (cut_power > threshold) {
                    output_detection_map[ant][d_cut][r_cut] = 1; // 检测到目标
                } else {
                    output_detection_map[ant][d_cut][r_cut] = 0; // 未检测到
                }
            }
        }
    }

    free(power_map);
    free(training_cell_powers);
    return 0; // 成功
}