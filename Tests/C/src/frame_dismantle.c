#include "sample_frame.h"
#include <stdio.h> // 用于输出

/**
 * 校验函数（简化，仅检查数据大小）
 */
static int frame_prepare_parse(const sample_frame_t *frame)
{
    // 简化检查：只检查数据长度，忽略 CRC 和帧头/尾
    if (frame->data_bytes == DATA_FIELD_BYTES) {
        return 0;
    }
    return -1;
}


/**
 * 函数名称: frame_data_dismantle
 * 功能描述: 将帧结构体中的原始一维数据重排为三维 int8_t 数组
 * 输入参数: 
 * frame: 完整的帧结构体指针
 * data: 目标三维数组 (Ant x Chirp x Point)
 * 返回说明: 0 成功, -1 失败
 */
int frame_data_dismantle(const sample_frame_t *frame, 
                        int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS])
{
    if (frame_prepare_parse(frame) != 0) {
        printf("Error: Frame prepare parse failed or data size mismatch.\n");
        return -1;
    }
    
    // 原始数据指针 (指向 struct data[] 的起始)
    const uint8_t *src_ptr = frame->data;
    uint32_t ant, chirp, point;
    uint32_t current_index = 0; // 原始数据流中的连续索引

    // 1. 遍历天线 (2根)
    for (ant = 0; ant < RADAR_ANT_COUNT; ant++) {
        // 2. 遍历 Chirp (64个)
        for (chirp = 0; chirp < RADAR_CHIRP_COUNT; chirp++) {
            // 3. 遍历采样点 (128个)
            for (point = 0; point < RADAR_CHIRP_POINTS; point++) {
                
                // 从原始 uint8_t 数组中取出数据
                uint8_t raw_value = src_ptr[current_index];
                
                // 转换为目标 int8_t 格式并存入三维数组
                // 注意：这里利用 C 语言的特性：(int8_t)raw_value 
                // 会自动将 uint8_t 的 128~255 映射为 int8_t 的 -128~-1 (双极性)
                data[ant][chirp][point] = (int8_t)raw_value;
                
                // 移动到下一个原始数据字节
                current_index++;
            }
        }
    }
    
    if (current_index != frame->data_bytes) {
        printf("Error: Data byte count mismatch after processing.\n");
        return -1;
    }
    
    return 0;
}