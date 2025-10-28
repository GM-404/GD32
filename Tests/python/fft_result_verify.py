import re
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# --- 配置区 ---
# 1. 设置 Matplotlib 字体以支持中文
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei']  # 尝试使用黑体
plt.rcParams['axes.unicode_minus'] = False  # 解决负号 '-' 显示为方块的问题

# 请将此路径替换为您的 radar_1DFFT_log.txt 文件的实际路径
LOG_FILE_PATH = "C:\\Users\\19513\\Desktop\\GD32\\bin\\Release\\radar_1DFFT_log.txt"

# 选择要绘制的天线索引
TARGET_ANT = 0  # 绘制天线 0 的所有 Chirp

# --- 解析函数 (用于提取所有 Chirp 的数据和元数据) ---

def parse_1dfft_log(log_path):
    """
    解析 radar_1DFFT_log.txt 文件，提取 FFT 幅值数据和元数据。
    返回:
    1. data (dict): { (ant, chirp): {'mag': [...]} }
    2. metadata (dict): 包含 RADAR_ANT_COUNT, RADAR_CHIRP_COUNT, HALF_POINTS
    """
    data = {}
    metadata = {}
    current_ant = -1
    current_chirp = -1
    
    # 正则表达式用于匹配元数据和 Range Bin 数据行
    meta_ant = re.compile(r'RADAR_ANT_COUNT: (\d+)')
    meta_chirp = re.compile(r'RADAR_CHIRP_COUNT: (\d+)')
    meta_range = re.compile(r'FFT Range Bins \(Output\): (\d+)')
    
    ant_chirp_pattern = re.compile(r'--- Ant (\d+) / Chirp (\d+) FFT Results ---')
    data_pattern = re.compile(r'Range Bin (\d+): R=([\d\.\-\+e]+), I=([\d\.\-\+e]+), Magnitude=([\d\.\-\+e]+)')
    
    with open(log_path, 'r') as f:
        for line in f:
            # 1. 提取元数据
            if meta_ant.search(line):
                metadata['ANT_COUNT'] = int(meta_ant.search(line).group(1))
            elif meta_chirp.search(line):
                metadata['CHIRP_COUNT'] = int(meta_chirp.search(line).group(1))
            elif meta_range.search(line):
                metadata['HALF_POINTS'] = int(meta_range.search(line).group(1))
            
            # 2. 匹配 Ant/Chirp 头部
            match_header = ant_chirp_pattern.search(line)
            if match_header:
                current_ant = int(match_header.group(1))
                current_chirp = int(match_header.group(2))
                key = (current_ant, current_chirp)
                data[key] = {'mag': []}
                continue
            
            # 3. 匹配数据行 (只提取幅值)
            match_data = data_pattern.search(line)
            if match_data and current_ant != -1:
                magnitude = float(match_data.group(4))
                key = (current_ant, current_chirp)
                if key in data:
                    data[key]['mag'].append(magnitude)
    
    return data, metadata

# --- 3D 绘图函数 ---

def plot_3d_range_chirp_map(all_fft_data, metadata, target_ant):
    """
    绘制指定天线的所有 Chirp 的 Range-Chirp Map 3D 表面图。
    """
    
    # 检查目标天线数据是否存在
    if target_ant not in [ant for ant, chirp in all_fft_data.keys()]:
        print(f"错误: 日志中未找到天线 {target_ant} 的数据。")
        return
        
    # 获取元数据
    CHIRP_COUNT = metadata.get('CHIRP_COUNT', 64)
    HALF_POINTS = metadata.get('HALF_POINTS', 64)
    
    # 构造二维数据矩阵 (Chirp x RangeBin)
    range_chirp_data = np.zeros((CHIRP_COUNT, HALF_POINTS))
    
    for chirp in range(CHIRP_COUNT):
        key = (target_ant, chirp)
        if key in all_fft_data and all_fft_data[key]['mag']:
            magnitudes = np.array(all_fft_data[key]['mag'])
            if len(magnitudes) == HALF_POINTS:
                range_chirp_data[chirp, :] = magnitudes
            # else: 忽略不完整的 Chirp

    # 转换为 dB 刻度作为 Z 轴高度
    Z = 20 * np.log10(range_chirp_data + 1e-6)
    
    # 创建 X (Range Bin) 和 Y (Chirp Index) 坐标
    x = np.arange(HALF_POINTS) # X轴: 距离频点
    y = np.arange(CHIRP_COUNT) # Y轴: Chirp 索引
    X, Y = np.meshgrid(x, y)
    
    # --- 绘制 3D 表面图 ---
    fig = plt.figure(figsize=(14, 10))
    # 创建 3D 坐标轴
    ax = fig.add_subplot(111, projection='3d')
    
    # 绘制表面图 (cmap 定义颜色，linewidth=0 消除网格线)
    # rstride 和 cstride 控制采样密度，对于大数据集可以增加
    surf = ax.plot_surface(X, Y, Z, cmap='viridis', 
                           rstride=1, cstride=1, 
                           antialiased=True, linewidth=0)
    
    # 设置标签和标题
    ax.set_title(f'天线 {target_ant} 1D FFT 幅值 3D 表面图', fontsize=16)
    ax.set_xlabel('距离频点 (Range Bin Index)')
    ax.set_ylabel('Chirp 索引')
    ax.set_zlabel('幅度 (dB)')
    
    # 调整视角 (可选，可以手动旋转)
    ax.view_init(elev=25, azim=-120) 
    
    # 添加颜色条
    fig.colorbar(surf, shrink=0.6, aspect=20, label='幅度 (dB)')

    plt.tight_layout()
    plt.show()
    
# --- 主程序执行 ---
if __name__ == '__main__':
    try:
        # 1. 解析数据和元数据
        all_fft_data, metadata = parse_1dfft_log(LOG_FILE_PATH)
        
        # 2. 绘制指定天线的所有 Chirp 的 3D 图
        plot_3d_range_chirp_map(all_fft_data, metadata, TARGET_ANT)
        
    except FileNotFoundError:
        print(f"错误: 找不到文件 {LOG_FILE_PATH}。请检查路径是否正确。")
    except Exception as e:
        print(f"处理文件时发生错误: {e}")