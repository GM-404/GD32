// radar_clutter_removal.c
#include "radar_clutter_removal.h"
#include <stdio.h>
#include <math.h> // For hypot

// ----------------------------------------------------
// 2. 独立去直流函数实现 
// ----------------------------------------------------
void perform_dc_removal(RadarFFT1DOutput data)
{
    double mean_re = 0.0;
    double mean_im = 0.0;
    const int NUM_RANGE_BINS = RADAR_CHIRP_POINTS / 2;
    // 总元素数量 (Re 和 Im 视为独立元素)
    const int num_elements = RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * NUM_RANGE_BINS;

    if (num_elements == 0) return;

    // 4a. 计算平均值 (求和)
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < NUM_RANGE_BINS; ++i) {
                mean_re += data[ant][chirp][i][0];
                mean_im += data[ant][chirp][i][1];
            }
        }
    }
    
    // 计算平均值
    mean_re /= num_elements;
    mean_im /= num_elements;

    // 4b. 减去平均值 (原地修改)
    // 对应 MATLAB 逻辑: dataFft1d = dataFft1d - mean(dataFft1d);
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < NUM_RANGE_BINS; ++i) {
                data[ant][chirp][i][0] -= mean_re;
                data[ant][chirp][i][1] -= mean_im;
            }
        }
    }
}