// radar_clutter_removal.h
#ifndef RADAR_CLUTTER_REMOVAL_H
#define RADAR_CLUTTER_REMOVAL_H

#include <stdint.h>
#include <fftw3.h> // 因为操作的是1D FFT后的复数数据
#include "private.h" // 包含雷达参数定义
#include "radar_1dfft.h" // 包含RadarFFT1DOutput类型定义


#define EN_RADAR_CLUTTER_REMOVAL   1
// ----------------------------------------------------
// 3. 独立去直流函数
// ----------------------------------------------------
/**
 * @brief 对 FFT 结果执行去直流操作 (减去平均值)。
 * @param data FFT结果数据 [Ant][Chirp][RangeBins/2][Re/Im] (将被原地修改)
 */
void perform_dc_removal(RadarFFT1DOutput data);
#endif // RADAR_CLUTTER_REMOVAL_H