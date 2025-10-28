import re
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# --- 配置区 ---
# 1. 设置 Matplotlib 字体以支持中文
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei']
plt.rcParams['axes.unicode_minus'] = False 

# 请将此路径替换为您的 radar_2DFFT_log.txt 文件的实际路径
# LOG_FILE_PATH = "radar_2DFFT_log.txt" 
LOG_FILE_PATH = "C:\\Users\\19513\\Desktop\\GD32\\bin\\Release\\radar_2DFFT_log.txt" # 如果需要指定绝对路径

# 选择要绘制的天线索引
TARGET_ANT = 0  # 绘制天线 0 的 Range-Doppler Map

# --- 解析函数 ---

def parse_2dfft_log(log_path):
    """
    解析 radar_2DFFT_log.txt 文件，提取 2D FFT 幅值数据和元数据。
    返回一个字典: { ant: {'mag': numpy_array (Doppler x Range)} }
    """
    data = {}
    metadata = {}
    current_ant = -1
    current_rbin = -1
    
    # 提取元数据
    meta_ant = re.compile(r'RADAR_ANT_COUNT: (\d+)')
    meta_doppler = re.compile(r'DOPPLER_POINTS: (\d+)')
    meta_range = re.compile(r'RANGE_BINS: (\d+)')
    
    # 匹配 Ant 头部
    ant_header_pattern = re.compile(r'=== Antenna (\d+) Results ===')
    # 匹配 Range Bin 头部 (用于切换距离门)
    rbin_header_pattern = re.compile(r'--- Ant (\d+) / Range Bin (\d+) Doppler FFT Results ---')
    # 匹配数据行 (只提取幅值)
    data_pattern = re.compile(r'Doppler Bin (\d+): R=([\d\.\-\+e]+), I=([\d\.\-\+e]+), Magnitude=([\d\.\-\+e]+)')
    
    # 临时存储：[ant][r_bin][d_bin_list]
    temp_data = {} 
    
    with open(log_path, 'r') as f:
        for line in f:
            # 1. 提取元数据
            m = meta_ant.search(line)
            if m: metadata['ANT_COUNT'] = int(m.group(1)); continue
            m = meta_doppler.search(line)
            if m: metadata['DOPPLER_POINTS'] = int(m.group(1)); continue
            m = meta_range.search(line)
            if m: metadata['RANGE_BINS'] = int(m.group(1)); continue
            
            # 2. 匹配 Antenna 头部
            m = ant_header_pattern.search(line)
            if m:
                current_ant = int(m.group(1))
                if current_ant not in temp_data:
                    temp_data[current_ant] = {}
                continue
            
            # 3. 匹配 Range Bin 头部
            m = rbin_header_pattern.search(line)
            if m and current_ant != -1:
                current_rbin = int(m.group(2))
                temp_data[current_ant][current_rbin] = []
                continue
            
            # 4. 匹配数据行
            m = data_pattern.search(line)
            if m and current_ant != -1 and current_rbin != -1:
                magnitude = float(m.group(4)) # 幅值 (group 4)
                if current_rbin in temp_data[current_ant]:
                    temp_data[current_ant][current_rbin].append(magnitude)
    
    # 整理数据为 NumPy 矩阵
    DOPPLER_POINTS = metadata.get('DOPPLER_POINTS', 64)
    RANGE_BINS = metadata.get('RANGE_BINS', 64)
    
    for ant, rbin_data in temp_data.items():
        # 初始化 [DopplerBins x RangeBins] 矩阵
        mag_matrix = np.zeros((DOPPLER_POINTS, RANGE_BINS)) 
        
        for r_bin, magnitudes in rbin_data.items():
            if r_bin < RANGE_BINS and len(magnitudes) == DOPPLER_POINTS:
                # MATLAB 习惯：Doppler 是行 (Chirp)，Range 是列 (Range Bin)
                mag_matrix[:, r_bin] = np.array(magnitudes)
        
        data[ant] = {'mag': mag_matrix}

    return data, metadata

# --- 3D 绘图函数 ---

def plot_3d_range_doppler_map(data, metadata, target_ant):
    """
    绘制指定天线的 Range-Doppler Map 3D 表面图。
    """
    if target_ant not in data:
        print(f"错误: 在日志中未找到天线 {target_ant} 的数据。")
        return
        
    # 获取幅值矩阵 (Doppler x Range)
    magnitude_matrix = data[target_ant]['mag']
    
    # 转换为 dB 刻度作为 Z 轴高度
    # Z 轴 (幅度): Doppler x Range
    Z = 20 * np.log10(magnitude_matrix + 1e-6)
    
    # 获取维度
    DOPPLER_POINTS, RANGE_BINS = Z.shape
    
    # 创建 X (Range Bin) 和 Y (Doppler Bin) 坐标
    x = np.arange(RANGE_BINS)    # X轴: 距离频点
    y = np.arange(DOPPLER_POINTS) # Y轴: 多普勒频点
    X, Y = np.meshgrid(x, y)
    
    # --- 绘制 3D 表面图 ---
    fig = plt.figure(figsize=(14, 10))
    # 创建 3D 坐标轴
    ax = fig.add_subplot(111, projection='3d')
    
    # 绘制表面图 (cmap 定义颜色，linewidth=0 消除网格线)
    surf = ax.plot_surface(X, Y, Z, cmap='viridis', 
                           rstride=1, cstride=1, 
                           antialiased=True, linewidth=0)
    
    # 设置标签和标题
    ax.set_title(f'天线 {target_ant} Range-Doppler Map 3D 表面图 (CFFT)', fontsize=16)
    ax.set_xlabel('距离频点 (Range Bin Index)')
    ax.set_ylabel('多普勒频点 (Doppler Bin Index)')
    ax.set_zlabel('幅度 (dB)')
    
    # 调整视角 (可选，可以手动旋转)
    ax.view_init(elev=30, azim=-140) 
    
    # 添加颜色条
    fig.colorbar(surf, shrink=0.6, aspect=20, label='幅度 (dB)')

    plt.tight_layout()
    plt.show()
    
# --- 主程序执行 ---
if __name__ == '__main__':
    try:
        # 1. 解析数据和元数据
        all_fft_data, metadata = parse_2dfft_log(LOG_FILE_PATH)
        
        # 2. 绘制指定天线的 3D 图
        plot_3d_range_doppler_map(all_fft_data, metadata, TARGET_ANT)
        
    except FileNotFoundError:
        print(f"错误: 找不到文件 {LOG_FILE_PATH}。请检查路径是否正确。")
    except Exception as e:
        print(f"处理文件时发生错误: {e}")