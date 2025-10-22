// radar_fft.c
#include "radar_1dfft.h"
#include <stdio.h>
#include <stdlib.h> 

int perform_1d_fft(const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                RadarFFT1DOutput output_fft_1d) {
    
    // FFTW 计划 (plan)
    fftw_plan plan;
    // 输入和输出数组
    fftw_complex *in;
    fftw_complex *out;

    // 为FFTW分配内存
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * RADAR_CHIRP_POINTS);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * RADAR_CHIRP_POINTS);

    if (in == NULL || out == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for FFTW input/output buffers.\n");
        fftw_free(in); // 尝试释放已分配的
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

    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            // 将int8_t数据转换为fftw_complex格式
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
                in[i][0] = (double)input_data[ant][chirp][i]; // 实部
                in[i][1] = 0.0;                                // 虚部 (因为原始数据是实数)
            }

            // 执行FFT
            fftw_execute(plan);

            // 将FFT结果存储到输出数组中
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
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