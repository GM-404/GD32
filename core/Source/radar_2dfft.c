// radar_2dfft.c

#include "radar_2dfft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 辅助函数：交换两个复数
static void swap_complex(double a[2], double b[2]) {
    double temp_re = a[0];
    double temp_im = a[1];
    a[0] = b[0];
    a[1] = b[1];
    b[0] = temp_re;
    b[1] = temp_im;
}

// 辅助函数：模拟 MATLAB 的 fftshift
// 对长度为 N 的 FFTW 输出数组进行原地 shift
static void fftshift_fftw(fftw_complex *out, int N) {
    // 假设 N 是偶数，如果 N 是奇数则需要稍微调整
    int k = N / 2;
    for (int i = 0; i < k; ++i) {
        // 交换前半部分和后半部分
        swap_complex(out[i], out[i + k]);
    }
}


void perform_2d_fft(const RadarFFT1DOutput input_1d_fft_data,
                RadarFFT2DOutput output_fft_2d) {
    
    fftw_plan plan;
    fftw_complex *in;
    fftw_complex *out;
    
    const int DOPPLER_SIZE = RADAR_CHIRP_COUNT;
    const int NUM_RANGE_BINS = RANGE_BINS; // rbinCut
    const int ANT_COUNT = RADAR_ANT_COUNT;

    // 为 FFTW 分配内存 (大小为 Chirp 数量)
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * DOPPLER_SIZE);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * DOPPLER_SIZE);

    if (in == NULL || out == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for 2D FFTW buffers.\n");
        fftw_free(in); 
        fftw_free(out);
    }

    // 创建一个1D FFT计划，针对 Chirp 维度 (Doppler)
    plan = fftw_plan_dft_1d(DOPPLER_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    if (!plan) {
        fprintf(stderr, "Error: Could not create 2D FFTW plan.\n");
        fftw_free(in);
        fftw_free(out);
    }
    
    // ** Doppler 窗函数已被移除 **
    
    
    // --- 2D FFT 主循环 (天线 -> 距离门) ---
    for (int ant = 0; ant < ANT_COUNT; ++ant) {
        for (int r_bin = 0; r_bin < RANGE_BINS; ++r_bin) {
            
            // 1. 提取 Chirp 维度数据 (不加窗)
            // 对应 MATLAB 逻辑: rangFft = dataFft1d(:,mm,lane);
            for (int chirp = 0; chirp < DOPPLER_SIZE; ++chirp) {
                // 读取 1D FFT 结果 (复数)
                double re = input_1d_fft_data[ant][chirp][r_bin][0];
                double im = input_1d_fft_data[ant][chirp][r_bin][1];
                
                // 不应用 Doppler 窗
                in[chirp][0] = re;
                in[chirp][1] = im;
            }

            // 2. 执行 FFT (Doppler 维)
            // 对应 MATLAB 逻辑: dopplerFftOri = fft(rangFft, N_chirp);
            fftw_execute_dft(plan, in, out);

            // 3. 执行 FFT Shift
            // 对应 MATLAB 逻辑: dopplerFft = fftshift(dopplerFftOri);
            fftshift_fftw(out, DOPPLER_SIZE);

            // 4. 存储结果到 2D FFT 输出数组
            // 对应 MATLAB 逻辑: dataFft2d(:,mm,lane) = dopplerFft.';
            for (int doppler = 0; doppler < DOPPLER_SIZE; ++doppler) {
                output_fft_2d[ant][doppler][r_bin][0] = out[doppler][0];
                output_fft_2d[ant][doppler][r_bin][1] = out[doppler][1];
            }
        }
    }

    // 5. 清理资源
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    printf("✅ 2D FFT (Range-Doppler) calculation completed (No Doppler Window).\n");
}