#ifndef RADAR_LOG_H
#define RADAR_LOG_H

#include "private.h"
#include <math.h>
#include <stdio.h>
#include "radar_1dfft.h"
#include "radar_2dfft.h"
#include "radar_cfar.h"
#include "radar_dbf.h"
#define EN_DISMANTLE_LOG         0
#define EN_1DFFT_LOG             1
#define EN_CLUTTER_REMOVAL_LOG   1
#define EN_2DFFT_LOG             1
#define EN_CFAR_LOG              1
#define EN_POINT_CLOUD_LOG       1

void radar_dismantle_log(const sample_frame_t *frame,int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS]); // 解析日志打印
void radar_1DFFT_log( RadarFFT1DOutput output_fft_1d);   // 1DFFT日志打印
void radar_clutter_removal_log(RadarFFT1DOutput output_fft_1d);   // 杂波去除日志打印
void radar_2DFFT_log(RadarFFT2DOutput output_fft_2d);   // 2DFFT日志打印
void radar_cfar_log(uint8_t detection_map[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RANGE_BINS]);  // CFAR日志打印
void print_cfar_detections_log(int frame_num, const DetectionInfo *detections, int count);   // 打印CFAR检测结果
void print_final_track_results(int frame_num, const Point *tracks, int count);     // 打印最终测角结果

#endif /* RADAR_LOG_H */