// radar_clutter_removal.c

#include "radar_clutter_removal.h"
#include <stdio.h>
#include <math.h>

/**
 * @brief 对1D FFT结果执行基于Chirp的直流杂波去除。
 * 该方法对每个天线、每个距离门，计算所有Chirp的平均值并减去。
 * @param data 1D FFT输出数据，维度为 [Ant][Chirp][RangeBins][Re/Im]。
 */
void perform_dc_removal(RadarFFT1DOutput data)
{
    // 检查是否启用了去直流
    if (!EN_RADAR_CLUTTER_REMOVAL) {
        return;
    }

    const int NUM_ANT = RADAR_ANT_COUNT;
    const int NUM_CHIRP = RADAR_CHIRP_COUNT;
    const int NUM_RANGE_BINS = RANGE_BINS;

    if (NUM_CHIRP == 0 || NUM_RANGE_BINS == 0) {
        fprintf(stderr, "Warning: FFT dimensions are zero, skipping DC removal.\n");
        return;
    }

    // 1. 遍历天线 (Lane)
    for (int ant = 0; ant < NUM_ANT; ++ant) {

        // 2. 遍历距离门 (Range Bins)
        // **注意：这个 Range Bin 循环必须在 Chirp 循环的外部**
        for (int i = 0; i < NUM_RANGE_BINS; ++i) {
            float sum_re = 0.0;
            float sum_im = 0.0;

            // a. 遍历Chirp (Doppler bins)，计算该 (Ant, Range) 上的平均值
            for (int chirp = 0; chirp < NUM_CHIRP; ++chirp) {
                sum_re += data[ant][chirp][i][0];
                sum_im += data[ant][chirp][i][1];
            }

            // b. 计算平均值 (Mean Range Profile for this specific Ant/Range bin)
            const float mean_re = sum_re / NUM_CHIRP;
            const float mean_im = sum_im / NUM_CHIRP;

            // c. 遍历Chirp，从每个元素中减去平均值 (原地修改)
            for (int chirp = 0; chirp < NUM_CHIRP; ++chirp) {
                data[ant][chirp][i][0] -= mean_re; // 实部减去平均实部
                data[ant][chirp][i][1] -= mean_im; // 虚部减去平均虚部
            }
        }
    }
    // printf("DC Clutter Removal successful.\n");
}