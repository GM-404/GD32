#include "radar_cfar.h"
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>   // For sqrt, log10
#include <stdbool.h>
#include <string.h> // For memset
#include <float.h>


CfarParams cfar_params = {
    .velDim = RADAR_CHIRP_COUNT,
    .rangeDim = RADAR_CHIRP_POINTS,
    .refCells = {CONFIG_CFAR_NUM_TRAIN_VEL, CONFIG_CFAR_NUM_TRAIN_RANGE},
    .guardCells = {CONFIG_CFAR_NUM_GUARD_VEL, CONFIG_CFAR_NUM_GUARD_RANGE},
    .thresholdFactor = CONFIG_CFAR_TH_AMP
};

// ... 其他函数实现 ...
// --- 辅助函数 ---

// 比较函数，用于qsort排序double数组
static int compare_doubles(const void *a, const void *b) {
    if (*(const double*)a < *(const double*)b) return -1;
    if (*(const double*)a > *(const double*)b) return 1;
    return 0;
}

// 辅助函数：找到四个数的中位数 (用于 median(refCellsData))
static double median_of_four(double a, double b, double c, double d) {
    double temp[4] = {a, b, c, d};
    // 排序
    qsort(temp, 4, sizeof(double), compare_doubles);
    // 返回排序后的第三个值 (索引 2) 作为中位数近似
    // (如果严格中位数是 (temp[1]+temp[2])/2，但 MatLab 逻辑通常取第三个值)
    return temp[2]; 
}

// 辅助函数：找到四个数中的最大值 (用于峰值分组)
static double max_of_four(double a, double b, double c, double d) {
    double m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    if (d > m) m = d;
    return m;
}

// 辅助函数：处理循环移位边界，返回合法的索引
// 注意：该函数返回的索引是 [0, size-1] 之间的，可以直接用于数组访问
static int get_circular_index(int index, int size) {
    // 确保索引在 [0, size-1] 范围内
    if (index >= size) {
        return index - size;
    }
    if (index < 0) {
        return index + size;
    }
    return index;
}


// --- 核心 CFAR 检测逻辑 (单个天线) ---

/**
 * @brief 核心二维CFAR检测，用于单个天线的功率图。
 * @param power_map 单个天线的 N x M 功率图。
 * @param N 速度维大小 (VEL_DIM)。
 * @param M 距离维大小 (RANGE_DIM)。
 * @param params CFAR参数配置。
 * @param out_detection_list 返回的详细检测列表指针地址。
 * @return int 检测到的目标数量。
 */
static int cfar2d_core(
    const double power_map[RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
    const int N, // VEL_DIM
    const int M, // RANGE_DIM
    const CfarParams *params,
    DetectionInfo **out_detection_list // 动态分配的列表
) 
{
    const int ref_vel = params->refCells[0];    
    const int ref_range = params->refCells[1]; 
    const int guard_vel = params->guardCells[0];
    const int guard_range = params->guardCells[1];
    const double thresholdFactor = params->thresholdFactor;

    int detCount = 0;
    int maxDetCount = 32; 
    DetectionInfo *detections = (DetectionInfo*)malloc(maxDetCount * sizeof(DetectionInfo));
    if (detections == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for detections.\n");
        *out_detection_list = NULL;
        return 0;
    }
    
    // MATLAB 逻辑的简化噪声估计 (用于峰值分组的快速排除)
    double noiseEstm = 0.0;
    for (int i = N - 6; i < N; i++) {
        for (int j = M - 6; j < M; j++) {
            noiseEstm += power_map[get_circular_index(i, N)][get_circular_index(j, M)];
        }
    }
    noiseEstm /= 36.0; 

    // 遍历：从 0-based 索引 1 到 倒数第 2 个索引 (避免边界检查的复杂性)
    // 距离维从 1 开始，排除 MATLAB 逻辑中的第 0 个距离 bin
    for (int r_idx = 1; r_idx < M - 1; r_idx++) { 
        for (int v_idx = 1; v_idx < N - 1; v_idx++) { 
            
            const double currentAmp = power_map[v_idx][r_idx];

            if (currentAmp <= 2.0 * noiseEstm) { // 快速排除
                continue;
            }

            // 峰值分组/局部最大值检查 (Peak Grouping)
            // MATLAB: data(velIdx-1, rangeIdx), data(velIdx+1, rangeIdx), ...
            double neighborMax = max_of_four(
                power_map[v_idx - 1][r_idx], power_map[v_idx + 1][r_idx],
                power_map[v_idx][r_idx - 1], power_map[v_idx][r_idx + 1]
            );
            if (currentAmp < neighborMax) {
                continue;
            }
            
            // --- 提取参考单元数据 (循环移位处理边界) ---
            double sum_up = 0.0, sum_down = 0.0;
            double sum_left = 0.0, sum_right = 0.0;
            
            // 速度维（上下）参考单元求和
            for (int i = 0; i < ref_vel; i++) {
                // 上侧参考单元 (Up): velIdx - guard_vel - ref_vel + i
                int index_up = v_idx - guard_vel - ref_vel + i;
                sum_up += power_map[get_circular_index(index_up, N)][r_idx];
                
                // 下侧参考单元 (Down): velIdx + guard_vel + 1 + i
                int index_down = v_idx + guard_vel + 1 + i;
                sum_down += power_map[get_circular_index(index_down, N)][r_idx];
            }
            
            // 距离维（左右）参考单元求和
            for (int i = 0; i < ref_range; i++) {
                // 左侧参考单元 (Left): rangeIdx - guard_range - ref_range + i
                int index_left = r_idx - guard_range - ref_range + i;
                sum_left += power_map[v_idx][get_circular_index(index_left, M)];
                
                // 右侧参考单元 (Right): rangeIdx + guard_range + 1 + i
                int index_right = r_idx + guard_range + 1 + i;
                sum_right += power_map[v_idx][get_circular_index(index_right, M)];
            }
            
            // --------------------------
            // 合并参考单元并估计噪声 (中位数估计)
            // --------------------------
            double mean_up = sum_up / ref_vel;
            double mean_down = sum_down / ref_vel;
            double mean_left = sum_left / ref_range;
            double mean_right = sum_right / ref_range;
            
            double noiseEst = median_of_four(mean_up, mean_down, mean_left, mean_right);
            
            // --------------------------
            // 检测判决与结果存储
            // --------------------------
            const double threshold = thresholdFactor * noiseEst;

            if (currentAmp > threshold) {
                // 动态数组扩展
                if (detCount >= maxDetCount) {
                    maxDetCount *= 2;
                    DetectionInfo *new_detections = (DetectionInfo*)realloc(detections, maxDetCount * sizeof(DetectionInfo));
                    if (new_detections == NULL) {
                        fprintf(stderr, "Error: Reallocation failed. Returning partial results.\n");
                        break; // 跳出内层循环
                    }
                    detections = new_detections;
                }

                // 信噪比计算 (20 * log10(幅度比))
                double snr = 0.0;
                if (noiseEst > DBL_MIN) { // 使用 DBL_MIN 避免除以零或极小值
                    snr = 20.0 * log10( currentAmp / noiseEst);
                } else {
                    snr = 99.0; 
                }
                
                // 存储结果
                detections[detCount].rangeIdx = r_idx;  // 0-based 距离索引
                detections[detCount].velIdx = v_idx;    // 0-based 速度索引
                detections[detCount].amplitude = currentAmp;
                detections[detCount].snr = snr;
                detections[detCount].noise = noiseEst;
                
                detCount++;
            }
        }
    }
    
    // 返回结果列表
    if (detCount == 0) {
        free(detections);
        *out_detection_list = NULL;
    } else {
        // 调整大小到实际检测到的数量
        DetectionInfo *final_detections = (DetectionInfo*)realloc(detections, detCount * sizeof(DetectionInfo));
        if (final_detections != NULL) {
            detections = final_detections;
        }
        *out_detection_list = detections;
    }
    
    return detCount;
}

// --- 外部接口实现 ---

/**
 * @brief 对2D FFT结果进行CFAR目标检测。
 */
int perform_cfar_detection(const RadarFFT2DOutput input_fft_2d_data,
                           RadarDetectionMap output_detection_map,
                           const CfarParams *params,
                           DetectionInfo **out_detection_info)
{
    // 检查参数合理性
    if (params == NULL) return -1;
    if (params->refCells[0] <= 0 || params->refCells[1] <= 0 || params->guardCells[0] < 0 || params->guardCells[1] < 0) {
        fprintf(stderr, "Error: CFAR parameters must be positive for refCells and non-negative for guardCells.\n");
        return -1;
    }
    
    // 1. 初始化检测图为0
    memset(output_detection_map, 0, RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS * sizeof(uint8_t));
    
    // 2. 遍历每个天线
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        
        // 临时存储功率图 (Matlab CFAR要求实数输入)
        double power_map[RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
        
        // a. 从 fftw_complex (double[2]) 转换为功率图 (幅度)
        for (int v = 0; v < RADAR_CHIRP_COUNT; ++v) {
            for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
                // fftw_complex[0] 是实部，[1] 是虚部
                double real = input_fft_2d_data[ant][v][r][0];
                double imag = input_fft_2d_data[ant][v][r][1];
                // 使用幅度作为 CFAR 输入 (幅度 = sqrt(R^2 + I^2))
                power_map[v][r] = sqrt(real * real + imag * imag);
            }
        }
        
        DetectionInfo *det_list = NULL;
        // 注意：这里我们使用核心CFAR函数，并要求它将详细检测信息写入 params->outputDetectionInfo
        // 我们假设调用者已经设置了 params->outputDetectionInfo 的指针地址。
        int det_count = cfar2d_core(
            power_map,
            RADAR_CHIRP_COUNT,
            RADAR_CHIRP_POINTS,
            params,
            &det_list // 接收详细检测列表
        );
        
        // b. 将详细检测结果映射到二进制检测图
        if (det_count > 0) {
            for (int i = 0; i < det_count; ++i) {
                int r_idx = det_list[i].rangeIdx;
                int v_idx = det_list[i].velIdx;
                
                // 确保索引在范围内
                if (r_idx < RADAR_CHIRP_POINTS && v_idx < RADAR_CHIRP_COUNT) {
                    output_detection_map[ant][v_idx][r_idx] = 1;
                }
            }
            // 释放核心函数分配的内存
            free(det_list);
        }
    }
    
    return 0; // 成功
}