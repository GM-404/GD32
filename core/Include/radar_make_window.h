#ifndef RADAR_MAKE_WINDOW_H
#define RADAR_MAKE_WINDOW_H

#include "private.h"


// 声明汉明窗数组，该数组将在外部定义 (在 hamming_window.c 中)
extern const double RADAR_HAMMING_WINDOW[RADAR_CHIRP_POINTS];

// 声明窗函数初始化函数
void initialize_hamming_window(void);

/**
 * @brief 应用窗函数到原始实数数据。
 * @param input_data 原始实数输入数据 [Ant][Chirp][Point]
 * @param output_data 输出的加窗后的实数数据 [Ant][Chirp][Point]
 * @param window 窗函数数组 (RADAR_CHIRP_POINTS 长度)
 */
void apply_windowing(const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                    double output_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
                    const double window[RADAR_CHIRP_POINTS]);

#endif /* RADAR_MAKE_WINDOW_H */