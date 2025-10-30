#ifndef RADAR_DBF_H
#define RADAR_DBF_H


#include "radar_cfar.h" // 包含 DetectionInfo
#include "private.h"    // 包含 RADAR_ANT_COUNT, RADAR_CHIRP_COUNT 等宏
#include <stdlib.h>
// 测角模块输出的结构体：只包含最终物理量
typedef struct {
    double r;           // 距离 (m)
    double v;           // 速度 (m/s)
    double a;           // 角度 (deg)
    double p;           // 幅度功率 (dB)
} TargetTrackInfo;

// 测角函数声明
int perform_dbf_estimation(
    const RadarFFT2DOutput input_fft_2d_data,
    const DetectionInfo *detections,
    int det_count,
    TargetTrackInfo **out_track_info // 用于传出新分配的结构体数组
);

// 独立日志打印函数声明
void print_final_track_results(int frame_num, const TargetTrackInfo *tracks, int count);

#endif // RADAR_DBF_H
