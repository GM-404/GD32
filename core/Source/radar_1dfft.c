// radar_1dfft.c
#include "radar_1dfft.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <math.h>   

// ----------------------------------------------------
// 1. FFT 主函数实现（只执行 FFT 和截断）
// ----------------------------------------------------
int perform_1d_fft(const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                RadarFFT1DOutput output_fft_1d) {
    
    fftw_plan plan;
    fftw_complex *in;
    fftw_complex *out;
    
    const int HALF_POINTS = RADAR_CHIRP_POINTS / 2;
    
    // 为FFTW分配内存
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * RADAR_CHIRP_POINTS);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * RADAR_CHIRP_POINTS);

    if (in == NULL || out == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for FFTW buffers.\n");
        fftw_free(in); 
        fftw_free(out);
        return -1;
    }

    // 创建一个1D FFT计划
    plan = fftw_plan_dft_1d(RADAR_CHIRP_POINTS, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    if (!plan) {
        fprintf(stderr, "Error: Could not create FFTW plan.\n");
        fftw_free(in);
        fftw_free(out);
        return -1;
    }
    
    // 循环：读取输入，执行 FFT，存储结果
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
                // ** 对应 MATLAB: chirpData = squeeze(atData(lane, chirp, :)); **
                // 将 int8_t 数据转换为 fftw_complex 格式 (虚部为 0)
                in[i][0] = (double)input_data[ant][chirp][i]; 
                in[i][1] = 0.0;
            }

            // ** 对应 MATLAB: rangFftOri = fft(chirpData, M_sample); **
            fftw_execute(plan);

            // ** 对应 MATLAB: rangFft = rangFftOri(1:M_sample/2); **
            // 将FFT结果的前一半存储到输出数组中
            for (int i = 0; i < HALF_POINTS; ++i) {
                output_fft_1d[ant][chirp][i][0] = out[i][0]; // 实部
                output_fft_1d[ant][chirp][i][1] = out[i][1]; // 虚部
            }
        }
    }

    // 清理FFTW资源
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return 0; // 成功
}


