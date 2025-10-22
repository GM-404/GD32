// radar_clutter_removal.h
#ifndef RADAR_CLUTTER_REMOVAL_H
#define RADAR_CLUTTER_REMOVAL_H

#include <stdint.h>
#include <fftw3.h> // 因为操作的是1D FFT后的复数数据
#include "private.h" // 包含雷达参数定义
#include "radar_1dfft.h" // 包含RadarFFT1DOutput类型定义

/**
 * @brief 对1D FFT后的数据进行静态杂波去除 (零多普勒DC分量去除)。
 *        该操作在Chirp维度上对每个天线、每个距离门进行均值减法。
 *
 * @param input_output_1d_fft_data 1D FFT后的复数数据。该数据将被修改为去除杂波后的结果。
 * @return 0 成功，-1 失败 (例如参数不合理)。
 */
int remove_static_clutter(RadarFFT1DOutput input_output_1d_fft_data);

#endif // RADAR_CLUTTER_REMOVAL_H