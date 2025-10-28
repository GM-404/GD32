// radar_fft_2d.h
#ifndef RADAR_FFT_2D_H
#define RADAR_FFT_2D_H

#include <stdint.h>
#include <fftw3.h> // FFTW 库头文件
#include "private.h" // 包含雷达参数定义
#include "radar_1dfft.h" // 包含1D FFT的输出类型定义 (RadarFFT1DOutput)

// 2D FFT的输入就是1D FFT的输出 (RadarFFT1DOutput)
// 定义2D FFT的输出数据类型：一个三维fftw_complex数组
typedef fftw_complex RadarFFT2DOutput[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RANGE_BINS];

// 用于存储峰值信息的结构体
typedef struct {
    double magnitude;
    int range_bin;
    int doppler_bin;
} PeakInfo;

/**
 * @brief 对1D FFT结果进行2D FFT处理。
 *        2D FFT在RADAR_CHIRP_COUNT维度上进行，即对每个天线、每个距离门进行FFT。
 *
 * @param input_1d_fft_data 1D FFT后的复数数据。
 * @param output_fft_2d 用于存储2D FFT结果的fftw_complex三维数组。
 *                      此数组应在调用前被分配好内存。
 * @return 0 成功，-1 失败 (例如FFTW计划创建失败或内存分配失败)。
 */
void perform_2d_fft(const RadarFFT1DOutput input_1d_fft_data,
                RadarFFT2DOutput output_fft_2d);

#endif // RADAR_FFT_2D_H