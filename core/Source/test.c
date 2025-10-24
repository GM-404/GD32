// main.c
#include <stdio.h>
#include <stdint.h>
#include <math.h> // For sqrt(), pow() to calculate magnitude for verification

#include "private.h"             // 包含雷达数据结构体定义
#include "radar_dismantle.h"     // 包含我们自己的头文件
#include "radar_1dfft.h"           // 包含1D FFT的头文件
#include "radar_clutter_removal.h" // 包含静态杂波去除的头文件 
#include "radar_2dfft.h"        // 包含2D FFT的头文件
#include "radar_cfar.h"          // 包含CFAR的头文件

// 1DFFT输入三维数组
static int8_t parsed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 1D FFT的输出，杂波去除的输入/输出，2D FFT的输入
static RadarFFT1DOutput fft_1d_output_data; 
// 2D FFT的输出，CFAR的输入
static RadarFFT2DOutput fft_2d_output_data;
// CFAR的输出：检测结果图
static RadarDetectionMap detection_map;

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
    // Let's create a signal that includes both static clutter and a moving target
    const double static_clutter_amplitude = 100.0; // 较强的静态杂波
    const double target_range_freq = 5.0; // 移动目标在距离门5
    const double target_doppler_freq = 2.0; // 移动目标在多普勒门2
    const double target_amplitude = 80.0; // 移动目标强度

    for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
        for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
                // 模拟静态杂波 (只与距离相关，不随chirp变化)
                double clutter_signal = static_clutter_amplitude * cos(2 * M_PI * target_range_freq * i / RADAR_CHIRP_POINTS);
                
                // 模拟移动目标
                double target_signal = target_amplitude * cos(
                                        2 * M_PI * target_range_freq * i / RADAR_CHIRP_POINTS +
                                        2 * M_PI * target_doppler_freq * chirp / RADAR_CHIRP_COUNT
                                    );
                
                // 叠加信号
                parsed_radar_data[ant][chirp][i] = (int8_t)(clutter_signal + target_signal);
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
    }

    // 3. 调用解析函数
    if (1) { // Assume dismantle successful for this example, or use your actual call
        printf("✅ frame_data_dismantle 解析成功 (or dummy data loaded).\n");

        // 4. 1DFFT
        printf("开始执行 1D FFT...\n");
        if (perform_1d_fft(parsed_radar_data, fft_1d_output_data) == 0) {
            printf("✅ 1D FFT 完成。\n");

            // 5. 静态杂波去除 <--- 新增步骤
            printf("\n开始执行 静态杂波去除...\n");
            if (remove_static_clutter(fft_1d_output_data) == 0) {
                printf("✅ 静态杂波去除完成。\n");
            } else {
                printf("❌ 静态杂波去除失败。\n");
            }

            // 6. 2DFFT
            printf("\n开始执行 2D FFT...\n");
            if (perform_2d_fft(fft_1d_output_data, fft_2d_output_data) == 0) {
                printf("✅ 2D FFT 完成。\n");

                // 7. CFAR 操作
                printf("\n开始执行 CFAR 目标检测...\n");
                CfarParams cfar_params = {
                    .guard_cells_range = 1,       // 距离维度保护单元 (单侧)
                    .guard_cells_doppler = 1,     // 多普勒维度保护单元 (单侧)
                    .training_cells_range = 2,    // 距离维度训练单元 (单侧)
                    .training_cells_doppler = 2,  // 多普勒维度训练单元 (单侧)
                    .threshold_factor = 1000.0,     // 阈值因子 (需要根据实际数据调整)
                    .cfar_strategy = 0            // 0: CA-CFAR (平均), 1: GO-CFAR (最大值)
                };

                if (perform_cfar_detection(fft_2d_output_data, detection_map, &cfar_params) == 0) {
                    printf("✅ CFAR 目标检测完成。\n");

                    // 打印CFAR检测结果 (示例)
                    printf("\nCFAR 检测结果示例 (Ant 0):\n");
                    int detected_count = 0;
                    for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
                        for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                            if (detection_map[0][d][r] == 1) {
                                detected_count++;
                                printf("  目标检测到: Ant %d, Range Bin %d, Doppler Bin %d\n", 0, r, d);
                            }
                        }
                    }
                    if (detected_count == 0) {
                        printf("  未检测到任何目标 (可能需要调整CFAR参数或信号太弱)。\n");
                    } else {
                        printf("  总共检测到 %d 个目标 (Ant 0)。\n", detected_count);
                    }
                    printf("  预期移动目标在 Range Bin %d, Doppler Bin %d 附近。\n", (int)target_range_freq, (int)target_doppler_freq);


                } else {
                    printf("❌ CFAR 目标检测失败。\n");
                }
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