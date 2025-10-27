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
    
    float avg_R, avg_I; // 用于存储每个距离门和天线的平均值

    // 1. 遍历每个天线
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        
        // 2. 遍历每个距离门
        for (int r_idx = 0; r_idx < RADAR_CHIRP_POINTS; ++r_idx) {
            
            // a. 计算该 (天线, 距离门) 组合在所有 Chirp 上的平均值 (DC 偏移)
            avg_R = 0.0;
            avg_I = 0.0;
            for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
                // 索引: [ant][chirp][r_idx][R/I]
                avg_R += input_output_1d_fft_data[ant][chirp][r_idx][0];
                avg_I += input_output_1d_fft_data[ant][chirp][r_idx][1];
            }
            avg_R /= RADAR_CHIRP_COUNT;
            avg_I /= RADAR_CHIRP_COUNT;

            // b. 从每个 Chirp 的数据中减去这个平均值
            for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
                input_output_1d_fft_data[ant][chirp][r_idx][0] -= avg_R; // 减去实部平均值
                input_output_1d_fft_data[ant][chirp][r_idx][1] -= avg_I; // 减去虚部平均值
            }
        }
    }
    
    //printf("✅ 静态杂波消除：DC 偏移减除法执行成功。\n");
    return 0;
}