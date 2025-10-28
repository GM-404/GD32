// radar_clutter_removal.c

#include "radar_clutter_removal.h"
#include <stdio.h>
#include <math.h>

/**
 * @brief 对 FFT 结果执行去直流操作 (减去所有元素的平均值)。
 * 对应 MATLAB 逻辑: dataFft1d = dataFft1d - mean(dataFft1d);
 */
void perform_dc_removal(RadarFFT1DOutput data)
{
    // 检查是否启用了去直流
    if (!EN_RADAR_CLUTTER_REMOVAL) {
        return;
    }

    double sum_re = 0.0;
    double sum_im = 0.0;
    const int NUM_RANGE_BINS = RADAR_CHIRP_POINTS / 2;
    
    // 总复数元素数量 (用于计算平均值)
    const long long total_elements = (long long)RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * NUM_RANGE_BINS;

    if (total_elements == 0) {
        fprintf(stderr, "Warning: Total FFT elements is zero, skipping DC removal.\n");
        return;
    }

    // 1. 遍历所有维度，计算所有元素的实部和虚部之和
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < NUM_RANGE_BINS; ++i) {
                // data[ant][chirp][i][0] 是实部
                // data[ant][chirp][i][1] 是虚部
                sum_re += data[ant][chirp][i][0];
                sum_im += data[ant][chirp][i][1];
            }
        }
    }
    
    // 2. 计算平均值 (mean_re + i * mean_im)
    const double mean_re = sum_re / total_elements;
    const double mean_im = sum_im / total_elements;

    // 3. 遍历所有维度，将平均值从每个元素中减去 (原地修改)
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < NUM_RANGE_BINS; ++i) {
                data[ant][chirp][i][0] -= mean_re; // 实部减去平均实部
                data[ant][chirp][i][1] -= mean_im; // 虚部减去平均虚部
            }
        }
    }

    printf("✅ DC removal successful. Subtracted mean (R=%.4f, I=%.4f) from all elements.\n", mean_re, mean_im);
}