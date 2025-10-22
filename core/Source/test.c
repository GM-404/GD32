// main.c
#include <stdio.h>
#include <stdint.h>
#include <math.h> // For sqrt() and pow() to calculate magnitude for verification
#include "radar_dismantle.h" // 包含我们自己的头文件
#include "private.h"         // 包含雷达数据结构体定义
#include "radar_fft.h"       // 包含1D FFT的头文件
#include "radar_fft_2d.h"    // 包含2D FFT的头文件

// 1DFFT输入三维数组
static int8_t parsed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 1D FFT的输出，2D FFT的输入
static RadarFFT1DOutput fft_1d_output_data; 
// 2D FFT的输出
static RadarFFT2DOutput fft_2d_output_data;

int main()
{
    // --- For demonstration, let's use a dummy packed_frame_data if not available ---
    #ifndef PACKED_FRAME_DATA_DEFINED
    const uint8_t dummy_packed_data[sizeof(sample_frame_t)] = {0}; // Create a dummy
    const sample_frame_t *frame = (const sample_frame_t *)dummy_packed_data;
    // Overwrite dummy frame head/tail for successful check in this example
    ((sample_frame_t *)dummy_packed_data)->frame_head = 0x55AA55BB;
    ((sample_frame_t *)dummy_packed_data)->frame_tail = 0x55CC55DD;

    // Fill dummy parsed_radar_data with some values for FFT test
    // Let's create a more interesting signal for 2D FFT visualization (e.g., a moving target)
    // A target at a certain range (peak in 1D FFT) and a certain velocity (peak in 2D FFT)
    const double target_range_freq = 5.0; // Corresponds to a peak at 5th bin in 1D FFT
    const double target_doppler_freq = 2.0; // Corresponds to a peak at 2nd bin in 2D FFT
    const double amplitude = 80.0;

    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
                // This combines a range frequency and a doppler frequency for visualization
                // Real part of a complex exponential: cos(2*pi*f_range*i + 2*pi*f_doppler*chirp)
                parsed_radar_data[ant][chirp][i] = (int8_t)(amplitude * cos(
                                                    2 * M_PI * target_range_freq * i / RADAR_CHIRP_POINTS +
                                                    2 * M_PI * target_doppler_freq * chirp / RADAR_CHIRP_COUNT
                                                ));
            }
        }
    }
    #else
    const sample_frame_t *frame = (const sample_frame_t *)packed_frame_data;
    #endif
    // --- End of dummy data section ---

    // 2. 验证帧头/尾 (Python 脚本是小端序，帧头是 BB 55 AA 55)
    if (frame->frame_head != 0x55AA55BB || frame->frame_tail != 0x55CC55DD) {
        printf("Warning: Frame header/tail check failed (Endianness or data issue).\n");
        // For actual data, you might want to exit here or handle error.
        // For dummy data, we proceed.
    }

    // 3. 调用解析函数
    if (1) { // Assume dismantle successful for this example, or use your actual call
        printf("✅ frame_data_dismantle 解析成功 (or dummy data loaded).\n");

        // 4. 1DFFT
        printf("开始执行 1D FFT...\n");
        if (perform_1d_fft(parsed_radar_data, fft_1d_output_data) == 0) {
            printf("✅ 1D FFT 完成。\n");

            // 验证1D FFT结果 (打印第一个天线，第一个Chirp的前几个点的幅度)
            printf("\n1D FFT 结果示例 (Ant 0, Chirp 0, 前10个点的幅度):\n");
            for (int i = 0; i < 10 && i < RADAR_CHIRP_POINTS; ++i) {
                double real = fft_1d_output_data[0][0][i][0];
                double imag = fft_1d_output_data[0][0][i][1];
                double magnitude = sqrt(real * real + imag * imag);
                printf("  Point %d: %lf\n", i, magnitude);
            }
            printf("  ...\n");

            // 5. 2DFFT
            printf("\n开始执行 2D FFT...\n");
            if (perform_2d_fft(fft_1d_output_data, fft_2d_output_data) == 0) {
                printf("✅ 2D FFT 完成。\n");

                // 验证2D FFT结果 (打印第一个天线，某个距离门的多普勒频谱幅度)
                // 通常我们会关注2D FFT结果中的峰值，它对应于目标的位置和速度。
                // 假设我们看第一个天线，距离门为 target_range_freq (即1D FFT的第5个bin)
                int range_bin_to_check = (int)target_range_freq;
                if (range_bin_to_check >= RADAR_CHIRP_POINTS) range_bin_to_check = 0; // Fallback

                printf("\n2D FFT 结果示例 (Ant 0, Range Bin %d, 前10个多普勒点的幅度):\n", range_bin_to_check);
                for (int i = 0; i < 10 && i < RADAR_CHIRP_COUNT; ++i) {
                    double real = fft_2d_output_data[0][i][range_bin_to_check][0];
                    double imag = fft_2d_output_data[0][i][range_bin_to_check][1];
                    double magnitude = sqrt(real * real + imag * imag);
                    printf("  Doppler Bin %d: %lf\n", i, magnitude);
                }
                printf("  ...\n");

                // 寻找最大峰值以验证
                double max_magnitude = 0.0;
                int max_range_bin = -1;
                int max_doppler_bin = -1;

                for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
                    for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                        double real = fft_2d_output_data[0][d][r][0];
                        double imag = fft_2d_output_data[0][d][r][1];
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
                
            } else {
                printf("❌ 2D FFT 失败。\n");
            }
        } else {
            printf("❌ 1D FFT 失败。\n");
        }
    }
    else {
        printf("❌ frame_data_dismantle 解析失败。\n");
    }

    // 清理FFTW全局资源
    fftw_cleanup();

    return 0;
}