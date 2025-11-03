// radar_1dfft.c - 使用 CMSIS-DSP 库实现 1D FFT

#include "radar_1dfft.h"
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include "arm_math.h" 
// --- CMSIS FFT 配置 ---
#define N_FFT_SIZE RADAR_CHIRP_POINTS 

// 定义 CMSIS CFFT 结构体实例 (必须使用 f32/float)
// 静态实例只需要初始化一次
static arm_cfft_instance_f32 S_1dfft; 

// CMSIS 要求输入/输出数据使用 R-I-R-I 交错存储格式，大小为 N_FFT_SIZE * 2
// 使用一个静态缓冲区来避免在堆上重复分配内存 (fftw_malloc)
// 缓冲区大小：点数 * 2 (实部+虚部)
static float fft_buffer_1d[N_FFT_SIZE * 2]; 


/**
 * @brief 初始化 1D FFT (距离 FFT) 的 CMSIS-DSP 结构体。
 * * 此函数只需要在系统启动或第一次运行 FFT 前调用一次。
 * * @return arm_status ARM_MATH_SUCCESS 表示成功。
 */
arm_status radar_1dfft_init(void)
{
    // arm_cfft_init_f32(实例指针, FFT点数)
    arm_status status = arm_cfft_init_f32(&S_1dfft, N_FFT_SIZE);
    
    if (status != ARM_MATH_SUCCESS) {
        fprintf(stderr, "Error: CMSIS 1D FFT initialization failed (Status: %d, Size: %d).\n", status, N_FFT_SIZE);
    }
    
    // 返回初始化状态
    return status;
}


/**
 * @brief 执行雷达数据的 1D FFT (距离 FFT)
 * * 使用 CMSIS-DSP 的 arm_cfft_f32 函数。
 * * @param input_data 原始 IQ 采样的输入数据（这里是实数输入）
 * @param output_fft_1d 1D FFT 结果输出（ComplexNum_f32 格式，只存储前一半）
 * @return int 0 表示成功
 */
void perform_1d_fft(const float input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                    RadarFFT1DOutput output_fft_1d) 
{
    const int HALF_POINTS = RADAR_CHIRP_POINTS / 2;
    
    // 循环：读取输入，执行 FFT，存储结果
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            
            // 1. 数据转换: 将输入数据转换为 CMSIS 要求的 R-I-R-I 交错格式
            // 假设你的输入 input_data 是实数（或只有实部）
            for (int i = 0; i < N_FFT_SIZE; i++) {
                fft_buffer_1d[2 * i]     = input_data[ant][chirp][i]; // 实部
                fft_buffer_1d[2 * i + 1] = 0.0f;                       // 虚部设为 0
            }
            
            // 2. 执行 CFFT 
            // CMSIS CFFT 是原地计算 (In-place)，结果写回 fft_buffer_1d
            // 参数1: 实例指针
            // 参数2: 数据缓冲区指针 (R-I-R-I)
            // 参数3: 反转标志 (0=FFT, 1=IFFT) -> 0 表示正向 FFT
            // 参数4: 位倒序标志 (通常设为 1，在 init 时已配置) -> 这个参数在 arm_cfft_f32 中不再使用，但旧的 arm_cfft_fast_f32 需要
            // 在 arm_cfft_f32 中，方向和位倒序都在 init 结构体中处理，但函数调用仍需要 direction 标志。
            // 为了安全，使用 arm_cfft_f32，它内部处理了位倒序。
            // CFFT 函数签名: void arm_cfft_f32(const arm_cfft_instance_f32 *S, float32_t *p1, uint8_t ifftFlag, uint8_t bitReverseFlag)
            // ifftFlag: 0 for forward FFT, 1 for inverse FFT
            // bitReverseFlag: 1 for bit reversal, 0 for normal output (通常用 1)
            arm_cfft_f32(&S_1dfft, fft_buffer_1d, 0, 1); 
            
            // 3. 结果存储: 将 CMSIS 交错格式的结果存储到输出数组中
            // 原始 FFTW 代码只存储了前一半的结果 (HALF_POINTS)
            for (int i = 0; i < HALF_POINTS; ++i) {
                // R-I 对应：2*i 和 2*i + 1
                output_fft_1d[ant][chirp][i][0] = fft_buffer_1d[2 * i];   // 实部
                output_fft_1d[ant][chirp][i][1] = fft_buffer_1d[2 * i + 1]; // 虚部
            }
            // 剩下的数据 (i >= HALF_POINTS) 被丢弃
        }
    }
}