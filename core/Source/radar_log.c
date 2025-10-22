#include "radar_log.h" 


void radar_dismantle_log(int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS])
{
    if (EN_DISMANTLE_LOG)
    {
    
    }
    else
    {
    }
}
void radar_1DFFT_log(RadarFFT1DOutput output_fft_1d)
{
    if (EN_1DFFT_LOG)
    {
    printf("✅ 1DFFT 成功解析数据。\n");
             // 验证1D FFT结果 (打印第一个天线，第一个Chirp的前几个点的幅度)
            printf("\n1D FFT 结果示例 (Ant 0, Chirp 0, 前10个点的幅度):\n");
            for (int i = 0; i < 10 && i < RADAR_CHIRP_POINTS; ++i) {
                double real = output_fft_1d[0][0][i][0];
                double imag = output_fft_1d[0][0][i][1];
                double magnitude = sqrt(real * real + imag * imag);
                printf("  Point %d: %lf\n", i, magnitude);
            }
            if (RADAR_CHIRP_POINTS > 10) {
                printf("  ...\n");
            }
            // 打印后10个点
            printf("1D FFT 结果示例 (Ant 0, Chirp 0, 后10个点的幅度):\n");
            for (int i = (RADAR_CHIRP_POINTS > 10 ? RADAR_CHIRP_POINTS - 10 : 0); i < RADAR_CHIRP_POINTS; ++i) {
                double real = output_fft_1d[0][0][i][0];
                double imag = output_fft_1d[0][0][i][1];
                double magnitude = sqrt(real * real + imag * imag);
                printf("  Point %d: %lf\n", i, magnitude);
            }
    }
    else
    {
    }
}