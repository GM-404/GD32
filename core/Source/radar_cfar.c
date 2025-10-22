// radar_cfar.c
#include "radar_cfar.h"
#include <stdio.h>
#include <stdlib.h> // For malloc, free, qsort
#include <math.h>   // For sqrt, fabs
#include <stdbool.h>
#include "private.h" // 包含雷达数据结构体定义
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
        fprintf(stderr, "Error: Invalid CFAR parameters.\n");
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
    // 十字形训练单元的最大数量
    // 沿着距离轴的训练单元： 2 * T_r * (2 * G_d + 1)
    // 沿着多普勒轴的训练单元： 2 * T_d * (2 * G_r + 1)
    // 理论上训练单元数量是固定的，计算准确值以避免不必要的内存浪费和溢出
    // 训练单元总数 = (2 * T_r * (2 * G_d + 1)) + (2 * T_d * (2 * G_r + 1))
    //  实际的十字形训练区通常不包括四角的交叉部分，所以是严格地沿着轴
    //  数量是： 2 * T_r * (2*G_d + 1)  (距离轴训练单元)
    //           + 2 * T_d * (2*G_r + 1)  (多普勒轴训练单元)
    // 假设是纯粹的十字形：
    int max_training_cells_per_arm_r = T_r * (2 * G_d + 1); // 一条距离臂上的训练单元数
    int max_training_cells_per_arm_d = T_d * (2 * G_r + 1); // 一条多普勒臂上的训练单元数
    int max_training_cells = 2 * (max_training_cells_per_arm_r + max_training_cells_per_arm_d); // 双臂

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
                
                // 遍历十字形参考窗口
                for (int r_offset = -(G_r + T_r); r_offset <= (G_r + T_r); ++r_offset) {
                    for (int d_offset = -(G_d + T_d); d_offset <= (G_d + T_d); ++d_offset) {
                        
                        // 跳过CUT自身
                        if (r_offset == 0 && d_offset == 0) continue;

                        int r_cell = r_cut + r_offset;
                        int d_cell = d_cut + d_offset;

                        // 检查边界
                        if (r_cell >= 0 && r_cell < RADAR_CHIRP_POINTS &&
                            d_cell >= 0 && d_cell < RADAR_CHIRP_COUNT) {
                            
                            // 定义保护区 (方形保护区)
                            bool in_guard_region_range = (abs(r_offset) <= G_r);
                            bool in_guard_region_doppler = (abs(d_offset) <= G_d);

                            // 如果在保护区内，跳过 (不用于噪声估计)
                            if (in_guard_region_range && in_guard_region_doppler) {
                                continue;
                            }

                            // 训练区（十字形，排除保护区）
                            // 沿着距离轴的训练单元 (在多普勒保护区内，且自身不在距离保护区内)
                            bool is_range_training = (abs(r_offset) > G_r && abs(r_offset) <= (G_r + T_r)) && in_guard_region_doppler;
                            // 沿着多普勒轴的训练单元 (在距离保护区内，且自身不在多普勒保护区内)
                            bool is_doppler_training = (abs(d_offset) > G_d && abs(d_offset) <= (G_d + T_d)) && in_guard_region_range;

                            if (is_range_training || is_doppler_training) {
                                if (current_training_cell_count < max_training_cells) {
                                    training_cell_powers[current_training_cell_count++] = power_map[ant][d_cell][r_cell];
                                } else {
                                    // 这是一个错误，表明 max_training_cells 计算得不够大
                                    fprintf(stderr, "Warning: Exceeded pre-allocated training_cell_powers buffer. Increase max_training_cells or adjust CFAR parameters.\n");
                                }
                            }
                        }
                    }
                }
                
                // 根据CFAR策略计算参考噪声
                double noise_estimate = 0.0;
                if (current_training_cell_count == 0) {
                    // 如果没有足够的参考单元，无法进行检测，跳过或设置为默认值
                    output_detection_map[ant][d_cut][r_cut] = 0;
                    continue; 
                }

                if (params->cfar_strategy == 0) { // CA-CFAR (平均)
                    double sum_noise_power = 0.0;
                    for (int i = 0; i < current_training_cell_count; ++i) {
                        sum_noise_power += training_cell_powers[i];
                    }
                    noise_estimate = sum_noise_power / current_training_cell_count;
                } else if (params->cfar_strategy == 1) { // GO-CFAR (最大值) 或 次大值
                    // 你的策略是“最大或者次大”，这里我们使用 qsort 排序
                    // 如果需要次大值，需要修改取值索引
                    if (current_training_cell_count > 0) {
                        qsort(training_cell_powers, current_training_cell_count, sizeof(double), compare_doubles);
                        
                        // 默认为最大值 (GO-CFAR)
                        noise_estimate = training_cell_powers[current_training_cell_count - 1]; 
                        
                        // 如果需要次大值，并且参考单元足够
                        // 可以在 CfarParams 中添加一个 int os_k_rank 参数，例如 1 为最大值，2 为次大值
                        // if (params->os_k_rank > 0 && current_training_cell_count >= params->os_k_rank) {
                        //     noise_estimate = training_cell_powers[current_training_cell_count - params->os_k_rank];
                        // }
                        
                    } else { // 这种情况应该被 current_training_cell_count == 0 捕获
                         output_detection_map[ant][d_cut][r_cut] = 0;
                         continue;
                    }
                } else {
                    fprintf(stderr, "Warning: Unknown CFAR strategy. Using default CA-CFAR.\n");
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