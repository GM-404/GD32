#ifndef RADAR_FFT_H
#define RADAR_FFT_H

#include <stdint.h>
#include <math.h>

// --- 假设从您的 radar_algo.h 中引入的宏定义 ---
#define RADAR_CHIRP_POINTS   (128)      // 单个 Chirp 的采样点数 N

// N点实数输入DFT的输出点数为 N/2 + 1
#define DFT_OUTPUT_COMPLEX_POINTS (RADAR_CHIRP_POINTS / 2 + 1) // 65 个复数点

// --- 复数结构体定义 ---
// 对应 CMSIS-DSP 的 float32_t 复数
typedef struct {
    float real;
    float imag;
} complex_float;


/**
 * @brief 初始化 1D DFT 所需的任何全局状态 (对于 DFT 而言是占位符)。
 * @return 0 成功
 */
int dft_init(void);

/**
 * @brief 执行单个 Chirp 数据的 1D 离散傅里叶变换 (DFT)。
 * * @param input_data 单个 Chirp 的 int8_t 采样数据 (RADAR_CHIRP_POINTS 个点)。
 * @param output_complex_bins 输出复数结果的缓冲区 (DFT_OUTPUT_COMPLEX_POINTS 个复数，共 65 点)。
 * @return 0 成功, -1 失败
 */
int dft_1d_process(const int8_t input_data[RADAR_CHIRP_POINTS], 
                complex_float output_complex_bins[DFT_OUTPUT_COMPLEX_POINTS]);

#endif // RADAR_FFT_H
