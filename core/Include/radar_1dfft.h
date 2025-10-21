// radar_fft.h
#ifndef RADAR_1DFFT_H
#define RADAR_1DFFT_H

#include <stdint.h>
#include <fftw3.h> // FFTW 库头文件
#include "private.h" // 包含雷达参数定义 (RADAR_ANT_COUNT, RADAR_CHIRP_COUNT, RADAR_CHIRP_POINTS)

// 定义1D FFT的输出数据类型：一个三维fftw_complex数组
// 注意：FFTW的fftw_complex实际上是一个double[2]的数组，表示实部和虚部
typedef fftw_complex RadarFFT1DOutput[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];

/**
 * @brief 对雷达中频信号数据进行1D FFT处理。
 *        FFT在RADAR_CHIRP_POINTS维度上进行。
 *
 * @param input_data 原始的int8_t雷达中频信号数据。
 * @param output_fft_1d 用于存储1D FFT结果的fftw_complex三维数组。
 *                      此数组应在调用前被分配好内存。
 * @return 0 成功，-1 失败 (例如FFTW计划创建失败)。
 */
int perform_1d_fft(const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                RadarFFT1DOutput output_fft_1d);

#endif // RADAR_FFT_H