// radar_cfar.h
#ifndef RADAR_CFAR_H
#define RADAR_CFAR_H

#include <stdint.h>
#include <fftw3.h> // 2D FFT的输出是fftw_complex
#include "private.h" // 包含雷达参数定义
#include "radar_2dfft.h" // 包含2D FFT的输出类型定义 (RadarFFT2DOutput)

// 定义CFAR的参数结构体
typedef struct {
    int guard_cells_range;
    int guard_cells_doppler;
    int training_cells_range;
    int training_cells_doppler;
    double threshold_factor;
    int cfar_strategy; // 0: CA-CFAR (CrossMean), 1: GO-CFAR (CrossMaxMean), 2: SO-CFAR (CrossMinMean), 3: OS-CFAR (CrossOS)
    int os_k_rank;     // For OS-CFAR: rank of the ordered statistic (e.g., 1 for max, 2 for second max)
} CfarParams;

extern CfarParams cfar_params; // 全局CFAR参数实例

// 定义CFAR的输出数据类型：一个三维uint8_t数组，1表示检测到目标，0表示未检测到
typedef uint8_t RadarDetectionMap[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];

/**
 * @brief 对2D FFT结果进行CFAR目标检测。
 *        使用十字形窗口，并可选择平均或最大值作为参考噪声。
 *
 * @param input_fft_2d_data 2D FFT后的复数数据（Range-Doppler谱）。
 * @param output_detection_map 用于存储检测结果的uint8_t三维数组。
 * @param params CFAR参数配置。
 * @return 0 成功，-1 失败 (例如参数不合理)。
 */
int perform_cfar_detection(const RadarFFT2DOutput input_fft_2d_data,
                        RadarDetectionMap output_detection_map,
                           const CfarParams *params);

#endif // RADAR_CFAR_H