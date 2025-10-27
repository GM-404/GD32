// radar_clutter_removal.c
#include "radar_clutter_removal.h"
#include <stdio.h>
#include <math.h> // For hypot

int remove_static_clutter(RadarFFT1DOutput input_output_1d_fft_data) {
    
    // 遍历每个天线和每个距离门
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int r_idx = 0; r_idx < RADAR_CHIRP_POINTS; ++r_idx) {
            // 对于当前天线和距离门，计算所有Chirp的平均值
            double sum_real = 0.0;
            double sum_imag = 0.0;

            for (int chirp_idx = 0; chirp_idx < RADAR_CHIRP_COUNT; ++chirp_idx) {
                sum_real += input_output_1d_fft_data[ant][chirp_idx][r_idx][0];
                sum_imag += input_output_1d_fft_data[ant][chirp_idx][r_idx][1];
            }

            double mean_real = sum_real / RADAR_CHIRP_COUNT;
            double mean_imag = sum_imag / RADAR_CHIRP_COUNT;

            // 从每个Chirp的数据中减去平均值
            for (int chirp_idx = 0; chirp_idx < RADAR_CHIRP_COUNT; ++chirp_idx) {
                input_output_1d_fft_data[ant][chirp_idx][r_idx][0] -= mean_real;
                input_output_1d_fft_data[ant][chirp_idx][r_idx][1] -= mean_imag;
            }
        }
    }
    
    return 0; // 成功
}
int remove_static_clutter_zero_doppler(RadarFFT1DOutput input_output_1d_fft_data) {
    
    // 零速通道的索引，通常是多普勒FFT结果的第一个bin (索引0)
    // 如果您的FFT输出是DC centered，零速通道可能在 RADAR_CHIRP_COUNT / 2
    // 但在雷达信号处理中，通常FFT的零频分量在索引0
    const int zero_doppler_bin = 32; 

    // 检查零速通道是否有效
    if (zero_doppler_bin >= RADAR_CHIRP_COUNT || zero_doppler_bin < 0) {
        fprintf(stderr, "Error: Invalid zero_doppler_bin index for clutter removal.\n");
        return -1; 
    }

    // 遍历每个天线和每个距离门
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int r_idx = 0; r_idx < RADAR_CHIRP_POINTS; ++r_idx) {
            // 将零速通道的数据（实部和虚部）直接置为零
            input_output_1d_fft_data[ant][zero_doppler_bin][r_idx][0] = 0.0; // 实部置零
            input_output_1d_fft_data[ant][zero_doppler_bin][r_idx][1] = 0.0; // 虚部置零
        }
    }
    
    //printf("✅ 静态杂波消除：零速通道置零法执行成功。\n");
    return 0; // 成功
}