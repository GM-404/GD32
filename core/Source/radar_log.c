#include "radar_log.h" 
#include <stdio.h> 
#define EN_SAVE_DISMANTLE_LOG 1 // 解析日志开关
void radar_dismantle_log(const sample_frame_t *frame, int8_t data[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RADAR_CHIRP_POINTS])
{
    if (EN_DISMANTLE_LOG)
    {
        FILE *log_file = fopen("radar_dismantle_log.txt", "w");
        if (log_file == NULL) {
            perror("Error opening log file");
            return; // 文件打开失败，退出函数
        }
         // 1.遍历CHIRP
    for (int ant = 0; ant < RADAR_ANT_COUNT; ant++) {
        // 2.遍历POINT
        for (int point = 0; point < RADAR_CHIRP_POINTS; point++) {
             // 3.遍历天线
            for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; chirp++) {
                //printf("Ant %d / Chirp %d / Point %d (Raw: %u, Parsed: %d)\n", ant, chirp, point, frame->data[ant * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS + chirp * RADAR_CHIRP_POINTS + point], data[ant][chirp][point]);
                fprintf(log_file, "Ant %d / Chirp %d / Point %d (Raw: %u, Parsed: %d)\n",
                    ant, chirp, point,
                    frame->data[ant * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS + chirp * RADAR_CHIRP_POINTS + point],
                    data[ant][chirp][point]);
            }
        }
    }
        // 关闭文件
        fclose(log_file);
        printf("Log data saved to radar_dismantle_log.txt\n"); // 提示用户文件已保存

        //单独看数据
        //point
        // for(int j = 0; j < 3; j++){
        //     //chirp
        //     for(int k = 0; k < RADAR_CHIRP_COUNT; k++){
        //         printf("Ant %d / Chirp %d / Point %d ( Parsed: %d)\n", 0, k, j,  data[0][k][j]);
        //     }
        // }
    }
    
}
void radar_1DFFT_log(RadarFFT1DOutput output_fft_1d)
{
    if (EN_1DFFT_LOG)
    {
        printf("✅ 1DFFT 结果解析中...\n");
        FILE *log_file = fopen("radar_1DFFT_log.txt", "w"); // 创建或覆盖日志文件
        if (log_file == NULL) {
            perror("Error opening 1D FFT log file"); 
            return; // 退出函数
        }

        // 定义正确的范围点数 (M_sample / 2)
        const int HALF_POINTS = RADAR_CHIRP_POINTS / 2; 
        
        fprintf(log_file, "--- 1D FFT Log Data (Full Output) ---\n");
        fprintf(log_file, "RADAR_ANT_COUNT: %d\n", RADAR_ANT_COUNT);
        fprintf(log_file, "RADAR_CHIRP_COUNT: %d\n", RADAR_CHIRP_COUNT);
        fprintf(log_file, "FFT Range Bins (Output): %d\n", HALF_POINTS);
        fprintf(log_file, "-------------------------------------\n\n");

        
        // 1. 遍历所有天线
        for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
            
            // 2. 遍历所有 Chirp
            for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
                
                fprintf(log_file, "--- Ant %d / Chirp %d FFT Results ---\n", ant, chirp);
                
                // 3. 遍历所有 Range Bin (M/2 点)
                for (int i = 0; i < HALF_POINTS; ++i) { 
                    
                    // 从四维数组中取出实部和虚部
                    double real = output_fft_1d[ant][chirp][i][0];
                    double imag = output_fft_1d[ant][chirp][i][1];
                    double magnitude = sqrt(real * real + imag * imag);

                    // 使用 .4f 精度打印
                    fprintf(log_file, "  Range Bin %d: R=%.4f, I=%.4f, Magnitude=%.4f\n",
                                    i, real, imag, magnitude);
                }
                fprintf(log_file, "\n"); // 每个 Chirp 块后加一个空行
            }
        }

        // 关闭文件
        fclose(log_file);
        printf("✅ 1D FFT 完整日志已保存至 radar_1DFFT_log.txt\n");
    }
}
void radar_clutter_removal_log(RadarFFT1DOutput output_fft_1d)
{
    if (EN_CLUTTER_REMOVAL_LOG)
    {
    printf("✅ 成功去除静态杂波\n");
     FILE *log_file = fopen("radar_clutter_removal_log.txt", "w"); // 创建或覆盖日志文件
        if (log_file == NULL) {
            perror("Error opening clutter removal log file"); // 文件打开失败时打印错误信息
            return; // 退出函数
        }

        fprintf(log_file, "--- Clutter Removal Log Data ---\n");
        fprintf(log_file, "RADAR_ANT_COUNT: %d\n", RADAR_ANT_COUNT);
        fprintf(log_file, "RADAR_CHIRP_COUNT: %d\n", RADAR_CHIRP_COUNT);
        fprintf(log_file, "RADAR_CHIRP_POINTS: %d\n", RADAR_CHIRP_POINTS);
        fprintf(log_file, "------------------------\n\n");


        // 遍历所有天线和所有 Chirp 的 1D FFT 结果
        //选择一个帧
        uint8_t chirp = 0;

        for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
            fprintf(log_file, "Ant %d / Chirp %d FFT Results:\n", ant, chirp);
            for (int i = 0; i < RADAR_CHIRP_POINTS; ++i) {
                double real = output_fft_1d[ant][chirp][i][0];
                double imag = output_fft_1d[ant][chirp][i][1];
                double magnitude = sqrt(real * real + imag * imag);

                fprintf(log_file, "  Point %d: R=%.2f, I=%.2f, Magnitude=%.2f\n",
                        i, real, imag, magnitude);
            }
            fprintf(log_file, "\n"); // 每个 Chirp 后加一个空行
        }

        // 关闭文件
        fclose(log_file);
        printf("clutter removal log data saved to radar_1DFFT_log.txt\n"); // 提示用户文件已保存
    }
}
void radar_2DFFT_log(RadarFFT2DOutput output_fft_2d)
{
    if (EN_2DFFT_LOG)
    {
    //printf("✅ 2DFFT 成功解析数据。\n");
    // 存储前3个最大峰值的信息
    const int NUM_TOP_PEAKS = 3;
    PeakInfo top_peaks[NUM_TOP_PEAKS];
    // 初始化峰值信息，幅度设为负无穷，确保任何合法幅度都能更新
    for (int i = 0; i < NUM_TOP_PEAKS; ++i) {
        top_peaks[i].magnitude = -1.0; // 幅度平方不会是负数，-1.0是安全的初始值
        top_peaks[i].range_bin = -1;
        top_peaks[i].doppler_bin = -1;
    }
    // // 寻找最大峰值以验证
    // double max_magnitude = 0.0;
    // int max_range_bin = -1;
    // int max_doppler_bin = -1;

    // for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
    //     for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
    //         double real = output_fft_2d[0][d][r][0];
    //         double imag = output_fft_2d[0][d][r][1];
    //         double magnitude = real * real + imag * imag; // 使用平方幅度，避免多次sqrt
    //         if (magnitude > max_magnitude) {
    //             max_magnitude = magnitude;
    //             max_range_bin = r;
    //             max_doppler_bin = d;
    //         }
    //     }
    // }
    // printf("\n2D FFT 检测到的最大峰值 (Ant 0):\n");
    // printf("  Range Bin: %d\n", max_range_bin);
    // printf("  Doppler Bin: %d\n", max_doppler_bin);
    // printf("  Magnitude: %lf\n", sqrt(max_magnitude)); // 打印实际幅度
    
    // 遍历所有单元格，找到前三个最大峰值
        for (int r = 0; r < RADAR_CHIRP_POINTS; ++r) {
            for (int d = 0; d < RADAR_CHIRP_COUNT; ++d) {
                // 我们只关心天线0的数据，如果需要所有天线，则需要增加一个循环
                double real = output_fft_2d[0][d][r][0];
                double imag = output_fft_2d[0][d][r][1];
                double current_magnitude_squared = real * real + imag * imag; // 使用平方幅度进行比较

                // 尝试将当前点插入到top_peaks数组中
                for (int i = 0; i < NUM_TOP_PEAKS; ++i) {
                    if (current_magnitude_squared > top_peaks[i].magnitude) {
                        // 找到一个更大的峰值，需要将当前点插入到i位置
                        // 将i及i之后的所有元素向后移动一位
                        for (int j = NUM_TOP_PEAKS - 1; j > i; --j) {
                            top_peaks[j] = top_peaks[j-1];
                        }
                        // 插入当前点
                        top_peaks[i].magnitude = current_magnitude_squared;
                        top_peaks[i].range_bin = r;
                        top_peaks[i].doppler_bin = d;
                        break; // 已插入，跳出内层循环
                    }
                }
            }
        }

        printf("\n2D FFT 检测到的前 %d 个最大峰值 (Ant 0):\n", NUM_TOP_PEAKS);
        for (int i = 0; i < NUM_TOP_PEAKS; ++i) {
            if (top_peaks[i].magnitude > 0) { // 只打印有效的峰值
                printf("  Peak %d:\n", i + 1);
                printf("    Range Bin: %d\n", top_peaks[i].range_bin);
                printf("    Doppler Bin: %d\n", top_peaks[i].doppler_bin);
                printf("    Magnitude: %lf\n", sqrt(top_peaks[i].magnitude)); // 打印实际幅度
            } else {
                printf("  Peak %d: (未检测到有效峰值)\n", i + 1);
            }
        }
    }
}
