#include <stdio.h>
#include <stdint.h>
#include <math.h> 
#include "radar_log.h"
#include "radar_dismantle.h" 
#include "private.h" 
#include "radar_1dfft.h"       
#include "radar_2dfft.h"
#include "radar_cfar.h" 

// 1DFFT输入三维数组
static int8_t parsed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 2DFFT输入三维数组
static RadarFFT1DOutput fft_1d_output_data; // 定义为静态全局变量，以避免栈溢出
// 2D FFT的输出
static RadarFFT2DOutput fft_2d_output_data;
// CFAR的输出：检测结果图
static RadarDetectionMap detection_map;

int main()
{
    // 1. 将原始字节流强制转换为结构体指针
    const sample_frame_t *frame = (const sample_frame_t *)packed_frame_data;

    // 2. 调用解析函数
    if (frame_data_dismantle(frame, parsed_radar_data) == 0) {
        radar_dismantle_log(frame, parsed_radar_data);
        // 3. 1DFFT
        if (perform_1d_fft(parsed_radar_data, fft_1d_output_data) == 0)
        {
            radar_1DFFT_log(fft_1d_output_data);
            // 4. 2DFFT
            if (perform_2d_fft(fft_1d_output_data, fft_2d_output_data) == 0)
            {
                printf("✅ 2DFFT 成功解析数据。\n");
                radar_2DFFT_log();
                // 5. CFAR
                printf("\n开始执行 CFAR 目标检测...\n");
                CfarParams cfar_params = {
                    .guard_cells_range = 1,       // 距离维度保护单元 (单侧)
                    .guard_cells_doppler = 1,     // 多普勒维度保护单元 (单侧)
                    .training_cells_range = 2,    // 距离维度训练单元 (单侧)
                    .training_cells_doppler = 2,  // 多普勒维度训练单元 (单侧)
                    .threshold_factor = 10.0,     // 阈值因子 (需要根据实际数据调整)
                    .cfar_strategy = 1            // 0: CA-CFAR (平均), 1: GO-CFAR (最大值)
                };
                if (perform_cfar_detection(fft_2d_output_data, detection_map, &cfar_params) == 0) {
                    printf("✅ CFAR 目标检测完成。\n");
                    printf("✅ CFAR 成功解析数据。\n");
                    //radar_CFAR_log();
                     // 打印CFAR检测结果 (示例)
                    // printf("\nCFAR 检测结果示例 (Ant 0):\n");
                    // int detected_count = 0;
                    // for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
                    //     for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                    //         if (detection_map[0][d][r] == 1) {
                    //             detected_count++;
                    //             printf("  目标检测到: Ant %d, Range Bin %d, Doppler Bin %d\n", 0, r, d);
                    //         }
                    //     }
                    // }
                }
                else
                {
                    printf("❌ CFAR 解析失败。\n");
                }
            }
            else
            {
                printf("❌ 2DFFT 解析失败。\n");
            }
        }
        else
        {
            printf("❌ 1DFFT 解析失败。\n");
        
        }
    }
    else{
        printf("❌ frame_data_dismantle 解析失败。\n");
    }
    return 0;
}
