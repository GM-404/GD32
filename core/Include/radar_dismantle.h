#ifndef RADAR_DISMANTLE_H
#define RADAR_DISMANTLE_H
#include <stdint.h> 
#include "private.h" // 包含 sample_frame_t 定义
// ------------------------------------
// 函数声明 
// ------------------------------------

// 解析函数
int frame_data_dismantle(const sample_frame_t *frame, int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS]);

#define PACKED_FRAME_SIZE 16412 
extern const uint8_t packed_frame_data[PACKED_FRAME_SIZE]; 
#endif /* RADAR_DISMANTLE_H */