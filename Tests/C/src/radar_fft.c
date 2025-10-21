#include "radar_fft.h"
#include <stdio.h> // 用于错误打印
#include <math.h>  // 确保 cosf 和 sinf 可用

// DFT 核心状态
static int is_initialized = 0;

/**
 * @brief 初始化 1D DFT 所需的任何全局状态。
 */
int dft_init(void)
{
    // 纯 DFT 实现不需要预计算表格
    is_initialized = 1;
    return 0;
}

/**
 * @brief 执行单个 Chirp 数据的 1D 离散傅里叶变换 (DFT)。
 * * 使用标准的 O(N^2) DFT 公式。
 */
int dft_1d_process(const int8_t input_data[RADAR_CHIRP_POINTS], 
                complex_float output_complex_bins[DFT_OUTPUT_COMPLEX_POINTS])
{
    if (!is_initialized) {
        if (dft_init() != 0) {
            fprintf(stderr, "Error: DFT not initialized.\n");
            return -1;
        }
    }

    const uint32_t N = RADAR_CHIRP_POINTS;
    const uint32_t K = DFT_OUTPUT_COMPLEX_POINTS; // N/2 + 1 = 65

    // DFT 公式: X[k] = sum_{n=0}^{N-1} x[n] * exp(-j * 2*pi*k*n / N)
    
    // 遍历输出频率点 k (从 0 到 N/2)
    for (uint32_t k = 0; k < K; k++) {
        float sum_real = 0.0f;
        float sum_imag = 0.0f;

        // 遍历输入时间点 n (从 0 到 N - 1)
        for (uint32_t n = 0; n < N; n++) {
            
            // 1. 输入数据 x[n] 转换为 float
            // 假设 input_data 已经是脚本转换后的 int8_t 范围值
            float x_n = (float)input_data[n];
            
            // 2. 计算角度: -2*pi*k*n / N
            // 使用 M_PI 或 3.1415926535f
            float angle = -2.0f * 3.1415926535f * (float)k * (float)n / (float)N;
            
            // 3. 计算旋转因子: exp(j*angle) = cos(angle) + j*sin(angle)
            float cos_term = cosf(angle);
            float sin_term = sinf(angle);
            
            // 4. 累加: X[k] += x[n] * (cos(angle) + j*sin(angle))
            sum_real += x_n * cos_term;
            sum_imag += x_n * sin_term;
        }

        // 存储结果
        output_complex_bins[k].real = sum_real;
        output_complex_bins[k].imag = sum_imag;
    }
    
    return 0;
}
