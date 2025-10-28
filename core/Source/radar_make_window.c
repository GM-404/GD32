// hamming_window.c

#include "radar_make_window.h"
#include <math.h>
#include <stdio.h>


static double s_hamming_window[RADAR_CHIRP_POINTS];

// 将静态数组的指针暴露给 extern 声明
const double *RADAR_HAMMING_WINDOW = s_hamming_window;


/**
 * @brief 根据 MATLAB 的 hamming(N) 公式计算汉明窗。
 * w(n) = 0.54 - 0.46 * cos(2*pi*n / (N-1))
 */
void initialize_hamming_window(void)
{
    const int N = RADAR_CHIRP_POINTS;
    const double N_minus_1 = (double)(N - 1);
    
    // 如果 N=1，则 w(0) = 1 (特殊情况)
    if (N <= 1) {
        if (N == 1) {
            s_hamming_window[0] = 1.0;
        }
        return;
    }

    for (int n = 0; n < N; ++n) {
        // 计算公式中的角度项
        double angle = 2.0 * M_PI * (double)n / N_minus_1; 
        
        // 计算汉明窗值
        s_hamming_window[n] = 0.54 - 0.46 * cos(angle);
        
        // 可选：打印几个值进行验证
        // printf("Hamming[%d] = %f\n", n, s_hamming_window[n]);
    }
    
    //printf("Hamming window (N=%d) initialized successfully.\n", N);
}
void apply_windowing(const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                    double output_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                    const double window[RADAR_CHIRP_POINTS])
{
    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
                // 将 int8_t 转换为 double 并应用窗函数
                output_data[ant][chirp][i] = (double)input_data[ant][chirp][i] * window[i];
            }
        }
    }
}