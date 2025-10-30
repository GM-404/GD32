// radar_cfar.h
#ifndef RADAR_CFAR_H
#define RADAR_CFAR_H

#include <stdint.h>
#include "arm_math.h"
#include "private.h" // 包含雷达参数定义 (如 RADAR_ANT_COUNT, RADAR_CHIRP_COUNT, RADAR_CHIRP_POINTS)
#include "radar_2dfft.h" // 包含2D FFT的输出类型定义 (RadarFFT2DOutput)
#include <math.h>
#include <float.h> // 使用 DBL_EPSILON
#include <stdbool.h>

// 定义CFAR的参数结构体
// 注意：移除了 'const' 关键字，并将逗号改为分号
typedef struct {
    int velDim;                     // 速度维大小
    int rangeDim;                   // 距离维大小
    int refCells[2];                // [速度维单侧参考单元数, 距离维单侧参考单元数]
    int guardCells[2];              // [速度维单侧保护单元数, 距离维单侧保护单元数]
    double thresholdFactor;         // 线性检测门限参数（倍数）
} CfarParams;

// 定义检测结果结构体
typedef struct {
    int rangeIdx;      // 距离维粗糙索引 (0-based)
    double rangeFine;  // 距离维精细值 (Interpolation result)
    int velIdx;        // 速度维粗糙索引 (0-based)
    double velFine;    // 速度维精细值 (Interpolation result)
    double amplitude;  // 检测点的幅度
    double snr;        // 信噪比 (dB)
    double noise;      // 噪声估计值
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

                        
/**
 * @brief 对峰值点进行二次插值，计算亚单元偏移量 delta。
 * * @param data_map 原始幅度图（我们使用平均幅度图：[VelDim][RangeDim]）。
 * @param peakIdx 峰值点在插值维度的索引（0基）。
 * @param fixedIdx 固定维度的索引（0基）。
 * @param isVelocityDim true=速度维插值，false=距离维插值。
 * @param dimSize 插值维度的总长度。
 * @return double 亚单元偏移量 delta。
 */
static double peak_interpolation_core(
    const double data_map[][RANGE_BINS],
    int peakIdx, 
    int fixedIdx, 
    bool isVelocityDim, 
    int dimSize);

/**
 * @brief 对CFAR检测到的目标进行二次插值细化（亚单元估计）。
 * * @param data_map 原始幅度图（平均幅度图：[VelDim][RangeDim]）。
 * @param detections 待细化的目标列表（原地修改）。
 * @param count 目标数量。
 * @param velDim 速度维大小 (RADAR_CHIRP_COUNT)。
 * @param rangeDim 距离维大小 (RANGE_BINS)。
 */
void refine_detections_interpolation(
    const double data_map[][RANGE_BINS],
    DetectionInfo *detections,
    int count,
    int velDim,
    int rangeDim);
#endif // RADAR_CFAR_H