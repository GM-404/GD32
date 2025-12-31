#include "radar_log.h" 
#include <stdio.h> 

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
        printf("Log data saved to radar_dismantle_log.txt\n"); 

        //单独看数据
        //point
        for(int j = 0; j < 3; j++){
            //chirp
            for(int k = 0; k < RADAR_CHIRP_COUNT; k++){
                printf("Ant %d / Chirp %d / Point %d ( Parsed: %d)\n", 0, k, j,  data[0][k][j]);
            }
        }
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
                    fprintf(log_file, "  Range Bin %d: R=%.2f, I=%.2f, Magnitude=%.2f\n",
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
    //宏
    if (EN_CLUTTER_REMOVAL_LOG) 
    {
        printf("✅ 去直流结果解析中...\n");
        // **修正 1: 修改日志文件名为 'radar_clutter_removal_log.txt'**
        FILE *log_file = fopen("radar_clutter_removal_log.txt", "w"); 
        if (log_file == NULL) {
            // **修正 2: 修正错误提示信息**
            perror("Error opening clutter removal log file"); 
            return; // 退出函数
        }

        // 定义正确的范围点数 (M_sample / 2)
        const int HALF_POINTS = RADAR_CHIRP_POINTS / 2; 
        
        // **修正 3: 修改日志文件头部描述**
        fprintf(log_file, "--- DC Removal Log Data (Post-Clutter Removal) ---\n");
        fprintf(log_file, "RADAR_ANT_COUNT: %d\n", RADAR_ANT_COUNT);
        fprintf(log_file, "RADAR_CHIRP_COUNT: %d\n", RADAR_CHIRP_COUNT);
        fprintf(log_file, "FFT Range Bins (Output): %d\n", HALF_POINTS);
        fprintf(log_file, "-------------------------------------\n\n");

        
        // 1. 遍历所有天线
        for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
            
            // 2. 遍历所有 Chirp
            for (int chirp = 0; chirp < RADAR_CHIRP_COUNT; ++chirp) {
                
                // **修正 4: 修改子标题描述**
                fprintf(log_file, "--- Ant %d / Chirp %d FFT Results (Post-DC) ---\n", ant, chirp);
                
                // 3. 遍历所有 Range Bin (M/2 点)
                for (int i = 0; i < HALF_POINTS; ++i) { 
                    
                    // 从四维数组中取出实部和虚部
                    double real = output_fft_1d[ant][chirp][i][0];
                    double imag = output_fft_1d[ant][chirp][i][1];
                    double magnitude = sqrt(real * real + imag * imag);

                    // 使用 .2f 精度打印
                    fprintf(log_file, "  Range Bin %d: R=%.2f, I=%.2f, Magnitude=%.2f\n",
                                    i, real, imag, magnitude);
                }
                fprintf(log_file, "\n"); // 每个 Chirp 块后加一个空行
            }
        }

        // 关闭文件
        fclose(log_file);
        // **修正 5: 修正最终控制台提示信息**
        printf("✅ 去直流完整日志已保存至 radar_clutter_removal_log.txt\n");
    }
}
void radar_2DFFT_log(RadarFFT2DOutput output_fft_2d)
{
    if (EN_2DFFT_LOG)
    {
        printf("✅ 2D FFT (Range-Doppler) 结果解析中...\n");
        // 创建或覆盖日志文件
        FILE *log_file = fopen("radar_2DFFT_log.txt", "w"); 
        if (log_file == NULL) {
            perror("Error opening 2D FFT log file"); 
            return; // 退出函数
        }

        // 定义正确的维度点数
        const int DOPPLER_POINTS = RADAR_CHIRP_COUNT;
        const int RANGE_BINS_LOG = RANGE_BINS; 
        
        // 写入日志文件头部信息
        fprintf(log_file, "--- 2D FFT Log Data (Range-Doppler Map) ---\n");
        fprintf(log_file, "RADAR_ANT_COUNT: %d\n", RADAR_ANT_COUNT);
        fprintf(log_file, "DOPPLER_POINTS: %d\n", DOPPLER_POINTS);
        fprintf(log_file, "RANGE_BINS_LOG: %d\n", RANGE_BINS_LOG);
        fprintf(log_file, "-------------------------------------------\n\n");

        
        // 1. 遍历所有天线
        for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
            
            fprintf(log_file, "=== Antenna %d Results ===\n", ant);
            
            // 为了方便 Python/MATLAB 解析热力图，我们按 Range Bin 组织
            // 2. 遍历所有 Range Bin (距离)
            for (int r_bin = 0; r_bin < RANGE_BINS_LOG; ++r_bin) {
                
                // 打印子标题：当前距离门的所有多普勒结果
                fprintf(log_file, "--- Ant %d / Range Bin %d Doppler FFT Results ---\n", ant, r_bin);
                
                // 3. 遍历所有 Doppler Bin
                for (int d_bin = 0; d_bin < DOPPLER_POINTS; ++d_bin) { 
                    
                    // 从四维数组中取出实部和虚部
                    double real = output_fft_2d[ant][d_bin][r_bin][0];
                    double imag = output_fft_2d[ant][d_bin][r_bin][1];
                    double magnitude = sqrt(real * real + imag * imag);

                    // 使用 .2f 精度打印
                    // 格式：Doppler Bin: R=..., I=..., Magnitude=...
                    fprintf(log_file, "  Doppler Bin %d: R=%.2f, I=%.2f, Magnitude=%.2f\n",
                                    d_bin, real, imag, magnitude);
                }
                fprintf(log_file, "\n"); // 每个 Range Bin 块后加一个空行
            }
        }

        // 关闭文件
        fclose(log_file);
        printf("✅ 2D FFT 完整日志已保存至 radar_2DFFT_log.txt\n");
    }
}
/**
 * @brief 打印 CFAR 二值检测图的日志。
 * @param detection_map CFAR 检测结果（二值图）
 */
void radar_cfar_log(uint8_t detection_map[RADAR_ANT_COUNT][RADAR_CHIRP_COUNT][RANGE_BINS])
{
    // if (EN_CFAR_LOG) 
    // {
    //     // 创建或覆盖日志文件
    //     FILE *log_file = fopen("radar_cfar_detection_log.txt", "w"); 
    //     if (log_file == NULL) {
    //         perror("Error opening CFAR detection log file"); 
    //         return; 
    //     }

    //     // 定义正确的维度点数
    //     const int DOPPLER_POINTS = RADAR_CHIRP_COUNT;
    //     const int RANGE_POINTS = RANGE_BINS; 
        
    //     // 写入日志文件头部信息
    //     fprintf(log_file, "--- CFAR Detection Map Log ---\n");
    //     fprintf(log_file, "RADAR_ANT_COUNT: %d\n", RADAR_ANT_COUNT);
    //     fprintf(log_file, "DOPPLER_POINTS: %d\n", DOPPLER_POINTS);
    //     fprintf(log_file, "RANGE_BINS_USED: %d\n", RANGE_POINTS);
    //     fprintf(log_file, "--------------------------------\n\n");

        
    //     // 1. 遍历所有天线
    //     for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
            
    //         fprintf(log_file, "=== Antenna %d Detections ===\n", ant);
    //         int ant_det_count = 0;

    //         // 2. 遍历所有 Range Bin
    //         for (int r_idx = 0; r_idx < RANGE_POINTS; ++r_idx) {
                
    //             // 3. 遍历所有 Doppler Bin
    //             for (int d_idx = 0; d_idx < DOPPLER_POINTS; ++d_idx) { 
                    
    //                 // 如果该单元检测到目标 (值为 1)
    //                 if (detection_map[ant][d_idx][r_idx] == 1) {
                        
    //                     // 记录目标位置 (Range-Doppler 坐标)
    //                     fprintf(log_file, "  Target Detected: Range Bin %d, Doppler Bin %d\n",
    //                                     r_idx, d_idx);
    //                     ant_det_count++;
    //                 }
    //             }
    //         }
            
    //         fprintf(log_file, "--- Antenna %d Total Detections: %d ---\n\n", ant, ant_det_count);
    //     }

    //     // 关闭文件
    //     fclose(log_file);
    // }
}
void print_cfar_detections_log(int frame_num, const DetectionInfo *detections, int count)
{
    if (!EN_CFAR_LOG ||count == 0 || detections == NULL) {
        // 如果没有目标，则不打印详细信息
        return;
    }
    
    // 打印帧头
    fprintf(stdout, "------> Frame = %d:\n", frame_num);
    fprintf(stdout, " Detected target information:\n");
    
    for (int i = 0; i < count; ++i) {
        fprintf(stdout, 
            "Target %d: Range=%d(%.2f), Velocity=%d(%.2f), Amplitude=%.2f, SNR=%.2fdB, NoiseEst=%.2f\n", 
            i + 1,                                       // 目标序号从 1 开始
            detections[i].rangeIdx, detections[i].rangeFine,
            detections[i].velIdx, detections[i].velFine,
            detections[i].amplitude,
            detections[i].snr,
            detections[i].noise
        );
    }
}
void print_final_track_results(int frame_num, const Point *tracks, int count)
{
    if (!EN_POINT_CLOUD_LOG || count == 0 || tracks == NULL) {
        fprintf(stdout, "==== Frame %d Final Track Log: No targets found. ====\n", frame_num);
        return;
    }
    
    fprintf(stdout, "================================================\n");
    fprintf(stdout, "==== Frame %d Final Track Log (R, V, A, P) ====\n", frame_num);
    fprintf(stdout, "------------------------------------------------\n");
    fprintf(stdout, "  Target | R (m) | V (m/s) | A (deg) | P (dB)\n");
    fprintf(stdout, "------------------------------------------------\n");
    
    for (int i = 0; i < count; ++i) {
        fprintf(stdout, 
            "  %5d  | %5.2f | %7.2f | %7.1f | %6.1f\n", 
            i + 1,
            tracks[i].r,
            tracks[i].v,
            tracks[i].a,
            tracks[i].p
        );
    }
}
