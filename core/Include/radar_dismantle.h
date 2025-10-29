#ifndef RADAR_DISMANTLE_H
#define RADAR_DISMANTLE_H
#include <stdint.h> 
#include "private.h"

// 解析函数
void frame_data_dismantle(const sample_frame_t *frame, int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS]);



#endif /* RADAR_DISMANTLE_H */