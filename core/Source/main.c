#include <stdio.h>
#include <stdint.h>
#include <math.h> 
#include "radar_dismantle.h" 
#include "private.h" // 包含雷达数据结构体定义
#include "radar_1dfft.h"       // 包含1D FFT的头文件

// 1DFFT输入三维数组
static int8_t parsed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 2DFFT输入三维数组
static RadarFFT1DOutput fft_1d_output_data; // 定义为静态全局变量，以避免栈溢出

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
            // 5. 2DFFT
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