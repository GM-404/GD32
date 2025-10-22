#include "radar_log.h" 


void radar_dismantle_log(const sample_frame_t *frame, int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS])
{
    if (EN_DISMANTLE_LOG)
    {
        // 验证点 1: 天线 0 / Chirp 0 / 第 0 个点 
        printf("Ant 0 / Chirp 0 / Point 0 (Raw: %u, Parsed: %d)\n", 
               frame->data[0], data[0][0][0]); // 预期: 255 和 -1

        // 验证点 2: 天线 0 / Chirp 0 / 第 1 个点 
        printf("Ant 0 / Chirp 0 / Point 1 (Raw: %u, Parsed: %d)\n", 
               frame->data[1], data[0][0][1]); // 预期: 3 和 3

        // 验证点 3: 天线 1 / 最后一个 Chirp / 最后一个点 
        uint32_t last_idx = frame->data_bytes - 1;
        printf("Ant 1 / Chirp 63 / Point 127 (Raw Index %u, Parsed: %d)\n", 
            last_idx,data[1][63][127]); 
            
        // 打印 Ant 0 的前 8 个 Chirp 的第一个点
        printf("\nAnt 0 / Chirp 0-7 / Point 0 (验证重排): \n");
        for (int i = 0; i < 8; i++) {
            printf("Chirp %d: %d\n", i, data[0][i][0]);
        }
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
}
void radar_2DFFT_log(RadarFFT2DOutput output_fft_2d)
{
    if (EN_2DFFT_LOG)
    {
    printf("✅ 2DFFT 成功解析数据。\n");
    // 寻找最大峰值以验证
    double max_magnitude = 0.0;
    int max_range_bin = -1;
    int max_doppler_bin = -1;

    for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
        for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
            double real = output_fft_2d[0][d][r][0];
            double imag = output_fft_2d[0][d][r][1];
            double magnitude = real * real + imag * imag; // 使用平方幅度，避免多次sqrt
            if (magnitude > max_magnitude) {
                max_magnitude = magnitude;
                max_range_bin = r;
                max_doppler_bin = d;
            }
        }
    }
    printf("\n2D FFT 检测到的最大峰值 (Ant 0):\n");
    printf("  Range Bin: %d\n", max_range_bin);
    printf("  Doppler Bin: %d\n", max_doppler_bin);
    printf("  Magnitude: %lf\n", sqrt(max_magnitude)); // 打印实际幅度
    }
}
