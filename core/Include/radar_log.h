#ifndef RADAR_LOG_H
#define RADAR_LOG_H

#include "private.h"
#include <math.h>
#include <stdio.h>
#include "radar_1dfft.h"


#define EN_DISMANTLE_LOG         0
#define EN_1DFFT_LOG             1
#define EN_2DFFT_LOG             0
#define EN_CFAR_LOG              0

void radar_dismantle_log(int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS]); // 解析日志打印
void radar_1DFFT_log( RadarFFT1DOutput output_fft_1d);   // 1DFFT日志打印

#endif /* RADAR_LOG_H */