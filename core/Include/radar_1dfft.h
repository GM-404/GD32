
#ifndef RADAR_1DFFT_H
#define RADAR_1DFFT_H

#include <stdint.h>
#include "arm_math.h"

#include "private.h" 



// 定义 FFT 1D 输出的类型，便于传递
// 维度: [Ant][Chirp][RangeBins][Re/Im]   传出来的是前半部分频谱
typedef float RadarFFT1DOutput[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS / 2][2];


//初始化
arm_status radar_1dfft_init(void);
/**
    * @brief 对输入的雷达数据执行1D FFT。
    * @param input_data 输入的雷达数据，维度为 [Ant][Chirp][Points]。
    * @param output_fft_1d 输出的1D FFT结果，维度为 [Ant][Chirp][Points/2][Re/Im]。
    * @return int 返回0表示成功，-1表示失败。
 */
void perform_1d_fft(const float input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                RadarFFT1DOutput output_fft_1d);


#endif // RADAR_FFT_H