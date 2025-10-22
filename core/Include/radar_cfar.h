// radar_cfar.h
#ifndef RADAR_CFAR_H
#define RADAR_CFAR_H

#include <stdint.h>
#include <fftw3.h> // 2D FFT的输出是fftw_complex
#include "private.h" // 包含雷达参数定义
#include "radar_2dfft.h" // 包含2D FFT的输出类型定义 (RadarFFT2DOutput)

// 定义CFAR的参数结构体
typedef struct {
    int guard_cells_range;   // 距离维度上的保护单元数量 (单侧)
    int guard_cells_doppler; // 多普勒维度上的保护单元数量 (单侧)
    int training_cells_range; // 距离维度上的参考单元数量 (单侧)
    int training_cells_doppler; // 多普勒维度上的参考单元数量 (单侧)
    double threshold_factor; // CFAR检测阈值因子 (alpha)
    // 根据你的“最大或次大”策略，可能需要额外的参数
    // 例如：int os_k_rank; // 如果使用OS-CFAR，表示取排序后第K个值
    int cfar_strategy; // 0: CA-CFAR (平均), 1: GO-CFAR (最大值)
} CfarParams;

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