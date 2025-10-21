#include "radar_log.h" 
#include "private.h"
#include <math.h>
#include <stdio.h>

void radar_1DFFT_log()
{
     printf("✅ 1DFFT 成功解析数据。\n");
             // 验证1D FFT结果 (打印第一个天线，第一个Chirp的前几个点的幅度)
            printf("\n1D FFT 结果示例 (Ant 0, Chirp 0, 前10个点的幅度):\n");
            for (int i = 0; i < 10 && i < RADAR_CHIRP_POINTS; ++i) {
                double real = fft_1d_output_data[0][0][i][0];
                double imag = fft_1d_output_data[0][0][i][1];
                double magnitude = sqrt(real * real + imag * imag);
                printf("  Point %d: %lf\n", i, magnitude);
            }
            if (RADAR_CHIRP_POINTS > 10) {
                printf("  ...\n");
            }
            // 打印后10个点
            printf("1D FFT 结果示例 (Ant 0, Chirp 0, 后10个点的幅度):\n");
            for (int i = (RADAR_CHIRP_POINTS > 10 ? RADAR_CHIRP_POINTS - 10 : 0); i < RADAR_CHIRP_POINTS; ++i) {
                double real = fft_1d_output_data[0][0][i][0];
                double imag = fft_1d_output_data[0][0][i][1];
                double magnitude = sqrt(real * real + imag * imag);
                printf("  Point %d: %lf\n", i, magnitude);
            }
}