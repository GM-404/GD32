// hamming_window.c

#include "radar_window.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// 静态数组：存储当前使用的窗函数值（仅用于 Hamming）
static double s_current_window[RADAR_CHIRP_POINTS];

/**
 * @brief  初始化选定的窗函数。必须在开始信号处理前调用一次。窗函数类型选择（矩形窗或汉明窗）。
 * 汉明窗:w(n) = 0.54 - 0.46 * cos(2*pi*n / (N-1))
 */
void initialize_windowing(WindowType type)
{
    const int N = RADAR_CHIRP_POINTS;
    
    // 如果是矩形窗或 N=1，则无需计算，直接返回。
    if (type == WINDOW_RECTANGULAR) {
        //printf("Windowing set to Rectangular (No Window) mode.\n");
        return;
    }
    if (N <= 1) {
        if (N == 1) {
            s_current_window[0] = 1.0;
        }
        return;
    }

    const double N_minus_1 = (double)(N - 1);

    for (int n = 0; n < N; ++n) {
        // 计算公式中的角度项
        double angle = 2.0 * M_PI * (double)n / N_minus_1;
        double cos_val = cos(angle);
        
        if (type == WINDOW_HAMMING) {
            // 汉明窗: w(n) = 0.54 - 0.46 * cos(angle)
            s_current_window[n] = 0.54 - 0.46 * cos_val;
        } 
        else {
            // 默认或未知类型，设置为 1.0 (等效于矩形窗)
            s_current_window[n] = 1.0; 
        }
    }
    
    printf("(N=%d) initialized successfully.\n",  N);
}
/**
 * @brief 将选定的窗函数应用于输入数据。
 * @param type 选定的窗类型。
 * @param input_data 原始 int8_t 数据。
 * @param output_windowed 加窗后的 double 数据。
 */
void apply_range_windowing(
    WindowType type,
    const int8_t input_data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS],
    double output_windowed[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS])
{
    const int N_ANT = RADAR_ANT_COUNT;
    const int N_CHIRP = RADAR_CHIRP_COUNT;
    const int N_POINTS = RADAR_CHIRP_POINTS;

    // 矩形窗（乘 1）的处理：只进行 int8_t 到 double 的类型转换
    if (type == WINDOW_RECTANGULAR) {
        for (int ant = 0; ant < N_ANT; ++ant) {
            for (int chirp = 0; chirp < N_CHIRP; ++chirp) {
                for (int i = 0; i < N_POINTS; ++i) {
                    // 仅进行类型转换，等效于乘 1
                    output_windowed[ant][chirp][i] = (double)input_data[ant][chirp][i];
                }
            }
        }
        return;
    }
    
    // 汉明窗的处理
    const double *window = s_current_window;

    for (int ant = 0; ant < N_ANT; ++ant) {
        for (int chirp = 0; chirp < N_CHIRP; ++chirp) {
            for (int i = 0; i < N_POINTS; ++i) {
                // 1. int8_t 转换为 double
                double data_val = (double)input_data[ant][chirp][i];
                
                // 2. 应用窗函数（乘法）
                output_windowed[ant][chirp][i] = data_val * window[i];
            }
        }
    }
}
