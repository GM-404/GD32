// radar_cfar.h
#ifndef RADAR_CFAR_H
#define RADAR_CFAR_H

#include <stdint.h>
#include <fftw3.h> // 2D FFT的输出是fftw_complex
#include "private.h" // 包含雷达参数定义 (如 RADAR_ANT_COUNT, RADAR_CHIRP_COUNT, RADAR_CHIRP_POINTS)
#include "radar_2dfft.h" // 包含2D FFT的输出类型定义 (RadarFFT2DOutput)

// 定义CFAR的参数结构体
// 注意：移除了 'const' 关键字，并将逗号改为分号
typedef struct {
    int velDim;                     // 速度维大小
    int rangeDim;                   // 距离维大小
    int refCells[2];                // [速度维单侧参考单元数, 距离维单侧参考单元数]
    int guardCells[2];              // [速度维单侧保护单元数, 距离维单侧保护单元数]
    double thresholdFactor;         // 线性检测门限参数（倍数）
    // 您可以选择在这里保留其他配置参数，但为了简洁，我们将其移除
    // char refCellSel;             // 例如：'median'
    // int peakGroupingEn;          // 例如：1
    // ...
} CfarParams;

// 定义检测结果结构体
typedef struct {
    int rangeIdx;      // 距离索引 (0-based)
    int velIdx;        // 速度索引 (0-based)
    double amplitude;  // 目标幅度
    double snr;        // 信噪比(dB)
    double noise;      // 噪声估计
} DetectionInfo;

extern CfarParams cfar_params; // 全局CFAR参数实例

// 定义CFAR的输出数据类型：一个三维uint8_t数组，1表示检测到目标，0表示未检测到
typedef uint8_t RadarDetectionMap[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RANGE_BINS];

/*
 * @brief 对2D FFT结果进行CFAR目标检测。
 *
 * @param input_fft_2d_data 2D FFT后的复数数据（Range-Doppler谱）。
 * @param output_detection_map 用于存储检测结果的uint8_t三维数组。
 * @param params CFAR参数配置。
 * @param out_detection_info 可选：用于返回详细检测列表（调用者负责释放）。
 * @return int 0 成功，-1 失败 (例如参数不合理)。
 */
int perform_cfar_detection(const RadarFFT2DOutput input_fft_2d_data,
                        RadarDetectionMap output_detection_map,
                         const CfarParams *params,
                         DetectionInfo **out_detection_info); // 将输出参数移到函数签名

#endif // RADAR_CFAR_H