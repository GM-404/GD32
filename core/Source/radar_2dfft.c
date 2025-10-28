// radar_fft_2d.c
#include "radar_2dfft.h"
#include <stdio.h>
#include <stdlib.h> 

int perform_2d_fft(const RadarFFT1DOutput input_1d_fft_data,
                RadarFFT2DOutput output_fft_2d) {
    
    // // FFTW 计划 (plan)
    // fftw_plan plan;
    // // 输入和输出数组
    // fftw_complex *in;
    // fftw_complex *out;

    // // 为FFTW分配内存
    // // 这里我们对RADAR_CHIRP_COUNT维度进行FFT，所以分配RADAR_CHIRP_COUNT大小的数组
    // in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * RADAR_CHIRP_COUNT);
    // out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * RADAR_CHIRP_COUNT);

    // if (in == NULL || out == NULL) {
    //     fprintf(stderr, "Error: Failed to allocate memory for 2D FFTW input/output buffers.\n");
    //     fftw_free(in); // 尝试释放已分配的
    //     fftw_free(out);
    //     return -1;
    // }

    // // 创建一个1D FFT计划，但是这次是对Chirp维度 (RADAR_CHIRP_COUNT) 进行
    // plan = fftw_plan_dft_1d(RADAR_CHIRP_COUNT, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // if (!plan) {
    //     fprintf(stderr, "Error: Could not create 2D FFTW plan.\n");
    //     fftw_free(in);
    //     fftw_free(out);
    //     return -1;
    // }

    // // 2D FFT是在每个天线、每个距离门上进行
    // for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
    //     for (int point = 0; point < RADAR_CHIRP_POINTS; ++point) {
    //         // 将输入数据（1D FFT的结果）填充到FFTW的输入缓冲区
    //         for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
    //             in[chirp][0] = input_1d_fft_data[ant][chirp][point][0]; // 实部
    //             in[chirp][1] = input_1d_fft_data[ant][chirp][point][1]; // 虚部
    //         }

    //         // 执行FFT (对当前天线、当前距离门的所有chirp进行)
    //         fftw_execute(plan);

    //         // 将FFT结果存储到输出数组中
    //         for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
    //             output_fft_2d[ant][chirp][point][0] = out[chirp][0]; // 实部
    //             output_fft_2d[ant][chirp][point][1] = out[chirp][1]; // 虚部
    //         }
    //     }
    // }

    // // 清理FFTW资源
    // fftw_destroy_plan(plan);
    // fftw_free(in);
    // fftw_free(out);

    // return 0; // 成功
}