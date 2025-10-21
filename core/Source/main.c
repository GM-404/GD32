#include <stdio.h>
#include <stdint.h>
#include <math.h> 
#include "radar_log.h"
#include "radar_dismantle.h" 
#include "private.h" // 包含雷达数据结构体定义
#include "radar_1dfft.h"       
#include "radar_2dfft.h"
// 1DFFT输入三维数组
static int8_t parsed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 2DFFT输入三维数组
static RadarFFT1DOutput fft_1d_output_data; // 定义为静态全局变量，以避免栈溢出
// 2D FFT的输出
static RadarFFT2DOutput fft_2d_output_data;
int main()
{
    // 1. 将原始字节流强制转换为结构体指针
    const sample_frame_t *frame = (const sample_frame_t *)packed_frame_data;
    
    // 2. 验证帧头/尾 (Python 脚本是小端序，帧头是 BB 55 AA 55)
    if (frame->frame_head != 0x55AA55BB || frame->frame_tail != 0x55CC55DD) {
        printf("Warning: Frame header/tail check failed (Endianness or data issue).\n");
    }

    // 3. 调用解析函数
    if (frame_data_dismantle(frame, parsed_radar_data) == 0) {
        
        // 4. 1DFFT
        if (perform_1d_fft(parsed_radar_data, fft_1d_output_data) == 0)
        {
            printf("✅ 1DFFT 成功解析数据。\n");
            //radar_1DFFT_log();
            // 5. 2DFFT
            if (perform_2d_fft(fft_1d_output_data, fft_2d_output_data) == 0)
            {
                printf("✅ 2DFFT 成功解析数据。\n");
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
