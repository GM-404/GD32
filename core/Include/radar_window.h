#ifndef RADAR_WINDOW_H
#define RADAR_WINDOW_H

#include "private.h"


// 窗函数类型选择
typedef enum {
    WINDOW_RECTANGULAR = 0, // 矩形窗（不加窗，乘1）
    WINDOW_HAMMING     = 1, // 汉明窗
} WindowType;

/**
 * @brief 初始化选定的窗函数。
 * 必须在开始信号处理前调用一次。
 * @param type 选定的窗类型。
 */
void initialize_windowing(WindowType type);

/**
 * @brief 将选定的窗函数应用于输入数据。
 * @param type 选定的窗类型。
 * @param input_data 原始 int8_t 数据。
 * @param output_windowed 加窗后的 double 数据。
 */
void apply_range_windowing(
    WindowType type,
    const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
    double output_windowed[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS]);

#endif // RADAR_WINDOW_H