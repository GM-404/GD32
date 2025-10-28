#include <stdio.h>
#include <stdint.h>
#include <math.h> 
#include "private.h" 
#include "radar_log.h"
#include "radar_dismantle.h" 
//#include "radar_make_window.h"
#include "radar_1dfft.h"  
#include "radar_clutter_removal.h"     
#include "radar_2dfft.h"
#include "radar_cfar.h" 
#include <stdbool.h>
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
    for (int frame_counter = 0; frame_counter < TOTAL_FRAMES_IN_ARRAY; ++frame_counter) {
        printf("\n=======================================================\n");
        printf("              处理第 %d 帧数据\n", frame_counter + 1);
        printf("=======================================================\n");
        // 1. 将原始字节流强制转换为结构体指针
        // 计算当前帧数据在 packed_frame_data 数组中的起始地址
        const uint8_t *current_frame_bytes = &packed_frame_data[frame_counter * PACKED_FRAME_SIZE];
        const sample_frame_t *frame = (const sample_frame_t *)current_frame_bytes;

        // 2. 调用解析函数
        if (frame_data_dismantle(frame, parsed_radar_data) == 0) {
            radar_dismantle_log(frame, parsed_radar_data);
            //3. 1DFFT
            if (perform_1d_fft(parsed_radar_data, fft_1d_output_data) == 0)
            {
                radar_1DFFT_log(fft_1d_output_data);
                // 4. 静态杂波消除
                perform_dc_removal(fft_1d_output_data);
                radar_clutter_removal_log(fft_1d_output_data);
                // 5. 2DFFT
                if (perform_2d_fft(fft_1d_output_data, fft_2d_output_data) == 0)
                {
                    radar_2DFFT_log(fft_2d_output_data);
                    // 6. CFAR
                    printf("\n开始执行 CFAR 目标检测...\n");
                    if (perform_cfar_detection(fft_2d_output_data, detection_map, &cfar_params, NULL) == 0) {
                        printf("---------------------------------------------------\n");
                        printf("✅ CFAR 检测成功。\n");
                        printf("---------------------------------------------------\n");
                        
                        // 打印CFAR检测结果 (Ant 0)
                        int detected_count = 0;
                        const int target_ant = 0; // 只打印天线 0 的结果

                        printf("CFAR 检测结果 (Ant %d):\n", target_ant);
                        
                        // 遍历距离维 (Range)
                        for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
                            
                            bool range_has_detection = false;
                            // 检查该距离门是否有目标
                            for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                                if (detection_map[target_ant][d][r] == 1) {
                                    range_has_detection = true;
                                    break;
                                }
                            }

                            if (range_has_detection) {
                                printf("  Range Bin %-3d 检测到目标，位于 Doppler Bin(s): [", r);
                                
                                // 遍历速度/多普勒维 (Doppler) 并打印检测到的点
                                for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                                    if (detection_map[target_ant][d][r] == 1) {
                                        printf("%d, ", d);
                                        detected_count++;
                                    }
                                }
                                // 打印目标结束
                                printf("\b\b]\n"); // 使用 \b\b 擦除最后的 ", "
                            }
                        }
                        
                        printf("\n总检测到的目标点数 (Ant %d): %d 个\n", target_ant, detected_count);
                        
                        // 如果需要更详细的表格打印，可以使用以下代码：
                        /*
                        printf("\n详细检测点列表:\n");
                        for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
                            for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                                if (detection_map[target_ant][d][r] == 1) {
                                    printf("  [Ant %d] R_Bin: %-3d, D_Bin: %-3d\n", target_ant, r, d);
                                }
                            }
                        }
                        */
                    } else {
                        printf("---------------------------------------------------\n");
                        printf("❌ CFAR 检测失败。\n");
                        printf("---------------------------------------------------\n");
                    }
                }
                else{
                    printf("❌ 2DFFT 解析失败。\n");
                }   
            }
            else{
                printf("❌ 1DFFT 解析失败。\n");
            }
        }
        else{
            printf("❌ frame_data_dismantle 解析失败。\n");
        }
    }
    return 0;
}
