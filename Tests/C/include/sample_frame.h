#ifndef SAMPLE_FRAME_H
#define SAMPLE_FRAME_H

#include <stdint.h>


#define RADAR_ANT_COUNT      (2)
#define RADAR_CHIRP_COUNT    (64)
#define RADAR_CHIRP_POINTS   (128)
#define DATA_FIELD_BYTES     (RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS) // 16384

/* 雷达采样信息结构体 */
typedef struct {
    uint32_t frame_head;         /* 帧头(0x55AA55BB) */
    uint32_t frame_index;        /* 雷达推送帧索引 */
    uint16_t curr_pack;          /* 当前包序号 */
    uint16_t total_pack;         /* 总数据包数 */
    uint8_t  data_type;          /* 数据帧类型(0:int8_t双极性ADC; ...) */
    uint8_t  samp_ants;          /* 采样天线数 */
    uint16_t data_crc16;         /* 数据源CRC16 */
    uint16_t samp_points;        /* 采样点数据 */
    uint16_t samp_chirps;        /* 采样chirp数 */
    uint32_t data_bytes;         /* 采样数据源字节数 */
    uint8_t  data[DATA_FIELD_BYTES]; /* 数据 */
    uint32_t frame_tail;         /* 帧尾(0x55CC55DD) */
} __attribute__((aligned(4))) sample_frame_t;

// 要测试的解析函数
int frame_data_dismantle(const sample_frame_t *frame, int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS]);

#endif // SAMPLE_FRAME_H