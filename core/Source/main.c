#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h> 
#include "private.h" 
//#include "radar_log.h"
#include "radar_dismantle.h" 
#include "radar_window.h"
#include "radar_1dfft.h"  
#include "radar_clutter_removal.h"     
#include "radar_2dfft.h"
#include "radar_cfar.h" 
#include "radar_dbf.h"
#include <stdbool.h>
#include <arm_math.h>

//加窗输入
static int8_t parsed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 1DFFT输入三维数组
static float windowed_radar_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS];
// 2DFFT输入三维数组
static RadarFFT1DOutput fft_1d_output_data; 
// CFAR的输入
static RadarFFT2DOutput fft_2d_output_data;
// CFAR的输出：检测结果图
static RadarDetectionMap detection_map;

int main()
{
    // 初始化窗口
    // initialize_windowing(WINDOW_HAMMING);

    // 用于接收详细的检测列表
    DetectionInfo *detailed_detections = NULL; 
    TargetTrackInfo *final_tracks = NULL;
    const int current_frame = 1; // 假设当前的帧序号   
    init_angle_scan_vector();
    // 1. 将原始字节流强制转换为结构体指针
    // 计算当前帧数据在 packed_frame_data 数组中的起始地址
    const sample_frame_t *frame = (const sample_frame_t *)packed_frame_data;

    // 2. 调用解析函数
    frame_data_dismantle(frame, parsed_radar_data);
    //radar_dismantle_log(frame, parsed_radar_data);

    // 3. 距离维加窗
    apply_range_windowing(WINDOW_HAMMING, parsed_radar_data, windowed_radar_data); 

    // 4. 1DFFT
    perform_1d_fft(windowed_radar_data, fft_1d_output_data);
    //radar_1DFFT_log(fft_1d_output_data);

    // 6. 静态杂波消除
    perform_dc_removal(fft_1d_output_data);
    //radar_clutter_removal_log(fft_1d_output_data);

    // 5. 速度维加窗
    apply_doppler_windowing(fft_1d_output_data);

    // 7. 2DFFT
    perform_2d_fft(fft_1d_output_data, fft_2d_output_data);
    //radar_2DFFT_log(fft_2d_output_data);

    // 8. CFAR(将从插值内化其中)
    int det_count = perform_cfar_detection(fft_2d_output_data, detection_map, &cfar_params, &detailed_detections);// 将指针的地址传入，接收检测列
    //radar_cfar_log(detection_map);
    //print_cfar_detections_log(current_frame, detailed_detections, det_count);

    // 9. 独立测角和物理量计算 (将 CFAR 结果转换为最终物理量列表)
    if (det_count > 0 && detailed_detections != NULL) {
    perform_dbf_estimation(fft_2d_output_data, detailed_detections, det_count, &final_tracks);
    }
    //释放指针
    if (detailed_detections != NULL) {
    free(detailed_detections);
    detailed_detections = NULL; 
    }
    // if (det_count > 0 && final_tracks != NULL) {
    // print_final_track_results(current_frame, final_tracks, det_count); 
    // }

    //清理最终结果内存
    if (final_tracks != NULL) {
        free(final_tracks);
        final_tracks = NULL; 
    }
    return 0;
}
