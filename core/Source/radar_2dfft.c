// radar_2dfft.c - 使用 CMSIS-DSP 库实现 2D FFT (Doppler FFT)

#include "radar_2dfft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// 引入 CMSIS DSP 库
#include "arm_math.h" 

// --- CMSIS FFT 配置 ---
#define N_DOPPLER_FFT_SIZE RADAR_CHIRP_COUNT 

// 定义 CMSIS CFFT 结构体实例 (静态实例只需要初始化一次)
static arm_cfft_instance_f32 S_2dfft; 

// CMSIS 要求输入/输出数据使用 R-I-R-I 交错存储格式，大小为 N_DOPPLER_FFT_SIZE * 2
// 我们使用一个静态缓冲区来避免在堆上重复分配内存
static float fft_buffer_2d[N_DOPPLER_FFT_SIZE * 2]; 

// -----------------------------------------------------------
// --- 辅助函数：针对 CMSIS R-I-R-I 交错格式的 FFT Shift ---
// -----------------------------------------------------------

/**
 * @brief 对 CMSIS R-I-R-I 交错格式的数组执行 FFT Shift
 * @param buffer 浮点数数组 (R-I-R-I 格式)
 * @param N 复数点数 (即 FFT_SIZE)
 */
static void fftshift_cmsis_f32(float *buffer, int N) {
    if (N % 2 != 0) {
        // 通常 FFT 点数是偶数，如果遇到奇数需要更复杂的处理，这里暂不实现
        fprintf(stderr, "Warning: FFT Shift with odd N is not implemented.\n");
        return;
    }
    
    // k 是要交换的块的长度 (N/2)
    int k = N / 2; 
    
    // 使用一个临时变量进行交换
    float temp_re, temp_im;
    
    // 只需要循环前半部分 k 个复数
    for (int i = 0; i < k; ++i) {
        // 索引 i 对应的复数在 buffer 中的位置: 2*i 和 2*i + 1
        // 索引 i+k 对应的复数在 buffer 中的位置: 2*(i+k) 和 2*(i+k) + 1
        
        int idx_a_re = 2 * i;
        int idx_a_im = 2 * i + 1;
        int idx_b_re = 2 * (i + k);
        int idx_b_im = 2 * (i + k) + 1;
        
        // 交换实部
        temp_re = buffer[idx_a_re];
        buffer[idx_a_re] = buffer[idx_b_re];
        buffer[idx_b_re] = temp_re;
        
        // 交换虚部
        temp_im = buffer[idx_a_im];
        buffer[idx_a_im] = buffer[idx_b_im];
        buffer[idx_b_im] = temp_im;
    }
}


/**
 * @brief 执行雷达数据的 2D FFT (Doppler FFT)
 * @param input_1d_fft_data 1D FFT 的复数结果
 * @param output_fft_2d 2D FFT 的复数结果输出
 * @return void
 */
void perform_2d_fft(const RadarFFT1DOutput input_1d_fft_data,
                    RadarFFT2DOutput output_fft_2d) {
    
    // 初始化 FFT 实例 (只需要做一次)
    static bool init_done = false;
    if (!init_done) {
        // 标准 CMSIS CFFT 初始化函数签名是: arm_cfft_init_f32(实例指针, FFT点数)
        arm_status status = arm_cfft_init_f32(&S_2dfft, N_DOPPLER_FFT_SIZE);

        if (status != ARM_MATH_SUCCESS) {
            fprintf(stderr, "Error: CMSIS 2D FFT initialization failed (status: %d).\n", status);
            return;
        }
        init_done = true;
    }

    const int DOPPLER_SIZE = RADAR_CHIRP_COUNT;
    const int ANT_COUNT = RADAR_ANT_COUNT;
    
    // 循环：天线 -> 距离门 (对 Chirp 维度执行 FFT)
    for (int ant = 0; ant < ANT_COUNT; ++ant) {
        // RANGE_BINS 是 1D FFT 结果的前一半点数
        for (int r_bin = 0; r_bin < RANGE_BINS; ++r_bin) { 
            
            // 1. 数据转换: 从 1D FFT 结果中提取 Doppler 维度数据到 CMSIS R-I-R-I 缓冲区
            for (int chirp = 0; chirp < DOPPLER_SIZE; ++chirp) {
                
                // --- 输入修复：通过指针访问实部和虚部 ---
                const float *p_complex_data = (const float *)&input_1d_fft_data[ant][chirp][r_bin];
                
                float re = p_complex_data[0]; // 实部
                float im = p_complex_data[1]; // 虚部
                
                // 存储为 R-I-R-I 交错格式
                fft_buffer_2d[2 * chirp]     = re;
                fft_buffer_2d[2 * chirp + 1] = im;
            }

            // 2. 执行 CFFT (Doppler 维)
            // CFFT 函数签名: arm_cfft_f32(实例指针, 数据缓冲区指针, ifftFlag, bitReverseFlag)
            arm_cfft_f32(&S_2dfft, fft_buffer_2d, 0, 1);

            // 3. 执行 FFT Shift
            // 确保 Doppler 零频位于中间
            fftshift_cmsis_f32(fft_buffer_2d, DOPPLER_SIZE);

            // 4. 存储结果到 2D FFT 输出数组
            for (int doppler = 0; doppler < DOPPLER_SIZE; ++doppler) {
                // --- 输出修复：通过指针写入实部和虚部 ---
                float *p_output_complex = (float *)&output_fft_2d[ant][doppler][r_bin];
                
                // R-I 对应：2*i 和 2*i + 1
                p_output_complex[0] = fft_buffer_2d[2 * doppler];     // 实部
                p_output_complex[1] = fft_buffer_2d[2 * doppler + 1]; // 虚部
            }
        }
    }

    printf("✅ 2D FFT (Range-Doppler) calculation completed with CMSIS-DSP.\n");
}
