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