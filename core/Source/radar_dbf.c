#include "radar_dbf.h"


double g_theta_scan[181];
int g_num_scan_angles;


// 初始化函数实现
void init_angle_scan_vector() {
    g_num_scan_angles = 0; 
    
    // 循环条件现在基于 NUM_THETA_SCAN 宏
    for (int i = 0; i < NUM_THETA_SCAN; ++i) {
        
        // 使用宏计算当前角度
        g_theta_scan[i] = START_ANGLE_DEG + (double)i * ANGLE_STEP_DEG; 
        
        g_num_scan_angles++;
    }
    
    // (可选) 确保 g_num_scan_angles 正确等于 NUM_THETA_SCAN
    if (g_num_scan_angles != NUM_THETA_SCAN) {
        // 如果这里触发，说明宏计算可能存在浮点误差，需要检查。
        fprintf(stderr, "Warning: Angle scan count mismatch! Calculated: %d, Macro: %d\n", 
                g_num_scan_angles, NUM_THETA_SCAN);
    }
}
// 辅助函数：复数乘法 result = a * b
static void complex_mult(const double a[2], const double b[2], double result[2]) {
    result[0] = a[0] * b[0] - a[1] * b[1]; // 实部：ac - bd
    result[1] = a[0] * b[1] + a[1] * b[0]; // 虚部：ad + bc
}

// 辅助函数：复数共轭点乘 result = a^H * X
// a 和 X 均为 N 元素复数向量 (double[N][2])
static void complex_dot_product_conj(const double a[RADAR_ANT_COUNT][2], 
                                    const double X[RADAR_ANT_COUNT][2], 
                                    double result[2]) 
{
    result[0] = 0.0;
    result[1] = 0.0;
    
    // Sum (a[i]^* * X[i])
    for (int i = 0; i < RADAR_ANT_COUNT; ++i) {
        // a[i]^* = a[i][0] - i*a[i][1]
        double a_conj[2] = {a[i][0], -a[i][1]}; 
        
        double term[2];
        complex_mult(a_conj, X[i], term);
        
        result[0] += term[0];
        result[1] += term[1];
    }
}


/**
 * @brief 基于FFT的DBF测角核心函数
 * @param sig_vector 天线接收数据 (RADAR_ANT_COUNT x 1，复数)
 * @param theta_scan 扫描角度向量 (度)
 * @param num_scan_angles 扫描角度数量
 * @param estimated_angle_deg 输出: 估计的角度 (度)
 * @param max_power_db 输出: 最大波束输出功率 (dB)
 * @return double 最大波束输出幅度 |P|
 */
static double DBF_core(
    const double sig_vector[RADAR_ANT_COUNT][2],
    const double theta_scan[],
    int num_scan_angles,
    double *estimated_angle_deg,
    double *dbf_max_power_db
) 
{
    const int N = RADAR_ANT_COUNT;
    const double d_lambda = 0.5; // d/lambda
    const double k_dl = 2.0 * C_PI * d_lambda;
    
    // 1. 生成并应用 Hamming 窗
    double win[N];
    double sum_win = 0.0;
    for (int n = 0; n < N; ++n) {
        win[n] = 0.54 - 0.46 * cos(2.0 * C_PI * n / (double)(N - 1));
        sum_win += win[n];
    }
    double norm_factor = (double)N / sum_win; // MATLAB 归一化因子
    
    double X_windowed[N][2];
    for (int i = 0; i < N; ++i) {
        X_windowed[i][0] = sig_vector[i][0] * win[i] * norm_factor;
        X_windowed[i][1] = sig_vector[i][1] * win[i] * norm_factor;
    }

    double max_P_abs_sq = -1.0;
    int max_idx = 0;
    
    for (int i = 0; i < num_scan_angles; ++i) {
        double theta_rad = theta_scan[i] * C_PI / 180.0;
        
        // 2. 计算导向矢量 'a'
        double a[N][2]; // 复数导向矢量
        for (int n = 0; n < N; ++n) {
            double phase = k_dl * (double)n * sin(theta_rad);
            a[n][0] = cos(phase); // 实部
            a[n][1] = sin(phase); // 虚部
        }
        
        // 3. 计算波束输出功率 P(i) = |a^H * X_windowed|
        double P_complex[2];
        complex_dot_product_conj(a, X_windowed, P_complex); 
        
        // 幅度平方: |P|^2 = R^2 + I^2
        double P_abs_sq = P_complex[0] * P_complex[0] + P_complex[1] * P_complex[1];
        
        if (P_abs_sq > max_P_abs_sq) {
            max_P_abs_sq = P_abs_sq;
            max_idx = i;
        }
    }
    
    // 4. 找到峰值并估计角度
    *estimated_angle_deg = theta_scan[max_idx];
    
    // 转换为 dB: 10 * log10(|P|^2)
    if (max_P_abs_sq > 0) {
         *dbf_max_power_db = 10.0 * log10(max_P_abs_sq);
    } else {
         *dbf_max_power_db = -99.9; // 极小值
    }

    return sqrt(max_P_abs_sq);
}

/**
 * @brief 对所有检测到的目标进行测角和物理量计算，并生成新的 TargetTrackInfo 数组。
 * * @param input_fft_2d_data 原始 2D FFT 结果 (所有天线，复数)
 * @param detections 粗糙目标列表 (包含 fine 索引)
 * @param det_count 目标数量
 * @param out_track_info 输出: 指向新分配的 TargetTrackInfo 数组的指针
 * @return int 返回检测到的目标数量
 */
int perform_dbf_estimation(
    const RadarFFT2DOutput input_fft_2d_data, // double[ANT][V][R][2]
    const DetectionInfo *detections,
    int det_count,
    TargetTrackInfo **out_track_info
)
{
    // 检查输入和初始化扫描向量
    if (det_count <= 0 || detections == NULL) {
        *out_track_info = NULL;
        return 0;
    }

    // 1. 分配新的 TargetTrackInfo 数组
    TargetTrackInfo *track_list = (TargetTrackInfo *)malloc(det_count * sizeof(TargetTrackInfo));
    if (track_list == NULL) {
        perror("Error allocating memory for TargetTrackInfo");
        *out_track_info = NULL;
        return 0;
    }

    const int N_chirp = RADAR_CHIRP_COUNT;
    const double vRes = RADAR_VELOCITY_RESOLUTION; // 假设已定义
    const double rRes = RADAR_RANGE_RESOLUTION;    // 假设已定义
    
    printf("Starting Angle Estimation (DBF) and physical conversion...\n");

    for (int i = 0; i < det_count; ++i) {
        const DetectionInfo *det = &detections[i];
        
        // 提取插值后的索引
        double velFine = det->velFine; 
        double rangeFine = det->rangeFine; 
        int velIdx = det->velIdx;
        int rangeIdx = det->rangeIdx;

        // 提取信号向量 (所有天线上同一 (R, V) 单元的复数数据)
        double sig_vector[RADAR_ANT_COUNT][2];
        for (int ant = 0; ant < RADAR_ANT_COUNT; ++ant) {
            sig_vector[ant][0] = input_fft_2d_data[ant][velIdx][rangeIdx][0]; 
            sig_vector[ant][1] = input_fft_2d_data[ant][velIdx][rangeIdx][1]; 
        }
        
        // 执行 DBF 测角 (返回角度和DBF功率)
        double estimated_angle = 0.0;
        double dbf_max_power_db = 0.0;
        
        // 假设 DBF_core 已实现
        DBF_core(
            sig_vector, 
            g_theta_scan, 
            g_num_scan_angles, 
            &estimated_angle, 
            &dbf_max_power_db
        );
    
        
        // 模拟 DBF 结果，实际应调用 DBF_core
        // (为确保编译通过，此处只使用 0.0)
        // estimated_angle = DBF_core(...)
        
        // 计算物理 R, V, P
        double v = (velFine - (double)N_chirp / 2.0) * vRes; // 速度
        double r = rangeFine * rRes;                           // 距离
        double power_db = 20.0 * log10(det->amplitude);        // 目标幅度功率 (dB)
        
        // 存储到 TargetTrackInfo 结构体
        track_list[i].r = r;
        track_list[i].v = v;
        track_list[i].a = estimated_angle; // 使用 DBF 结果
        track_list[i].p = power_db; 
    }
    
    *out_track_info = track_list;
    return det_count;
}
