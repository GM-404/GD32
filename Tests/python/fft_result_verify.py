import numpy as np
import sys
import os
import re
import matplotlib.pyplot as plt # 引入绘图库

# --- 配置区 ---

# 1. 设置 Matplotlib 字体以支持中文
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei']  # 尝试使用黑体，如果不行则使用微软雅黑
plt.rcParams['axes.unicode_minus'] = False  # 解决负号 '-' 显示为方块的问题

# 1. C 语言输出日志文件路径 (必须是固定的，用于对比)
# 
C_LOG_FILEPATH = "C:\\Users\\19513\\Desktop\\GD32\\bin\\Release\\radar_1DFFT_log.txt" 
C_LOG_FILEPATH = "C:\\Users\\19513\\Desktop\\GD32\\bin\\Release\\radar_clutter_removal_log.txt" 
# 2. Python 输出文件路径 (用于保存 Python 计算结果)
OUTPUT_DIRECTORY = "C:\\Users\\19513\\Desktop\\GD32\\data" 
PYTHON_OUTPUT_FILENAME = "fft_python_output.txt"
PYTHON_OUTPUT_FILEPATH = os.path.join(OUTPUT_DIRECTORY, PYTHON_OUTPUT_FILENAME)
PLOT_FILENAME = "fft_comparison_plot.png" # 绘图保存文件名
PLOT_FILEPATH = os.path.join(OUTPUT_DIRECTORY, PLOT_FILENAME)

# 3. 允许的最大误差 (浮点数比较，如果差值大于这个值则标记为失败)
MAX_ERROR_TOLERANCE = 0.01 

# --- 测试数据 ---
# 1. C 语言的输入数据 (Ant 0 / Chirp 0 的 128 个点)
rfft_test_input = np.array([
    -1, 3, -16, -31, -41, -19, -23, -26,
    -12, -34, -36, -26, -18, -23, -28, -31,
    -16, -8, -22, -21, -37, -36, -6, -1,
    -10, -15, -14, -11, 2, -1, -36, -23,
    13, 19, 29, -10, -36, -27, -2, 39,
    23, -11, 0, -3, -14, 3, 20, -2,
    3, 2, -18, -5, 8, 0, 9, 4,
    -21, -35, -8, 7, 21, 4, -25, -39,
    -41, -21, 25, 5, -1, -14, -23, -40,
    7, 12, -2, 9, -10, -21, -18, -18,
    17, 6, -11, -7, -9, -19, 30, 11,
    1, -1, -5, -21, 8, 25, 17, -7,
    -5, -10, -11, -1, 7, 13, -32, -22,
    11, 2, 29, 21, -31, -29, -15, 8,
    29, 16, -21, -25, -29, -13, -8, 0,
    0, -4, 2, -15, -35, -38, -17, -1,
], dtype=np.float32)

N = len(rfft_test_input)

# 正则表达式用于从 C 语言日志中提取 Point X, R=Y, I=Z
# 注意：C Log 中同时包含了 Magnitude，但我们只提取 R 和 I。
LOG_REGEX = re.compile(r"Point\s+(\d+):\s+R=([-]?[\d.]+),\s+I=([-]?[\d.]+),\s+Magnitude=([\d.]+)")

def run_python_fft(input_data):
    """计算 Python FFT，并保存结果到文件，同时返回复数结果列表。"""
    os.makedirs(OUTPUT_DIRECTORY, exist_ok=True)
    
    fft_output_py_complex = np.fft.fft(input_data)
    python_results = []

    # 1. 将结果保存到文件 (保持原始功能)
    with open(PYTHON_OUTPUT_FILEPATH, 'w', encoding='utf-8') as f:
        f.write("--- Python Output: FFT 结果 (N=128) ---\n")
        
        # 2. 收集结果用于对比
        for d_bin in range(N):
            Z = fft_output_py_complex[d_bin]
            R = Z.real
            I = Z.imag
            Mag = np.abs(Z) # 计算模值
            python_results.append({'R': R, 'I': I, 'Mag': Mag})
            f.write(f"Point {d_bin}: R={R:.2f}, I={I:.2f}\n")

    print(f"✅ Python FFT results saved to '{PYTHON_OUTPUT_FILEPATH}'")
    return python_results

def parse_c_log(filepath):
    """从 C 语言日志中解析 FFT 结果 (只读取 Ant 0 / Chirp 0 的结果)。"""
    c_results = {}
    found_target_section = False
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            for line in f:
                # 检查是否进入了目标段落
                if "Ant 0 / Chirp 0 FFT Results:" in line:
                    found_target_section = True
                    continue
                
                # 检查是否离开目标段落 (如果有其他 Ant/Chirp 的结果紧随其后)
                if found_target_section and line.startswith("Ant "):
                    break
                    
                if found_target_section:
                    match = LOG_REGEX.search(line)
                    if match:
                        index = int(match.group(1))
                        # 从日志中提取 R, I, Magnitude
                        R = float(match.group(2))
                        I = float(match.group(3))
                        Mag = float(match.group(4))
                        c_results[index] = {'R': R, 'I': I, 'Mag': Mag}
        
        if not c_results:
            raise ValueError("无法从日志文件中解析出任何 FFT 结果。请检查文件内容和格式。")
            
    except FileNotFoundError:
        print(f"❌ 错误: 找不到 C 语言日志文件: '{filepath}'")
        sys.exit(1)
    except ValueError as e:
        print(f"❌ 错误: 日志解析失败: {e}")
        sys.exit(1)
        
    print(f"✅ C 语言日志 '{filepath}' 解析成功，共 {len(c_results)} 个点。")
    return c_results

def plot_comparison(py_res, c_res_dict):
    """生成并保存 R, I, Magnitude 的对比图。"""
    
    # 准备绘图数据
    points = np.arange(N)
    
    # Python 数据
    py_R = np.array([d['R'] for d in py_res])
    py_I = np.array([d['I'] for d in py_res])
    py_Mag = np.array([d['Mag'] for d in py_res])

    # C Log 数据 (从字典中按顺序提取)
    # 我们假设 C Log 包含了 N 个点，并且 key 是 0 到 N-1
    c_R = np.array([c_res_dict[i]['R'] for i in range(N) if i in c_res_dict])
    c_I = np.array([c_res_dict[i]['I'] for i in range(N) if i in c_res_dict])
    c_Mag = np.array([c_res_dict[i]['Mag'] for i in range(N) if i in c_res_dict])
    
    # 绘图设置
    fig, axes = plt.subplots(3, 1, figsize=(14, 12), sharex=True)
    plt.suptitle("FFT 结果对比 (C 语言 vs Python NumPy)", fontsize=16)

    # 1. 绘制实部 (R)
    axes[0].plot(points, py_R, 'b-', label='Python R', alpha=0.7)
    axes[0].plot(points, c_R, 'r--', label='C Log R', alpha=0.7)
    axes[0].set_ylabel("Real Part (R)")
    axes[0].set_title("实部 (R) 对比")
    axes[0].grid(True, linestyle=':', alpha=0.6)
    axes[0].legend()
    
    # 2. 绘制虚部 (I)
    axes[1].plot(points, py_I, 'b-', label='Python I', alpha=0.7)
    axes[1].plot(points, c_I, 'r--', label='C Log I', alpha=0.7)
    axes[1].set_ylabel("Imaginary Part (I)")
    axes[1].set_title("虚部 (I) 对比")
    axes[1].grid(True, linestyle=':', alpha=0.6)
    axes[1].legend()

    # 3. 绘制模值 (Magnitude)
    axes[2].plot(points, py_Mag, 'b-', label='Python Mag', alpha=0.7)
    axes[2].plot(points, c_Mag, 'r--', label='C Log Mag', alpha=0.7)
    axes[2].set_ylabel("Magnitude")
    axes[2].set_title("模值 (Magnitude) 对比")
    axes[2].set_xlabel("FFT Point (Bin Index)")
    axes[2].grid(True, linestyle=':', alpha=0.6)
    axes[2].legend()
    
    # 调整布局，防止重叠
    plt.tight_layout(rect=(0, 0.03, 1, 0.96))
    # 保存图片
    plt.savefig(PLOT_FILEPATH)
    print(f"✅ 对比图已保存到: '{PLOT_FILEPATH}'")
    plt.show() # 显示绘图窗口

# ... (compare_results 函数与原始脚本功能相同，此处省略，请使用您提供的完整脚本)

def compare_results(py_res, c_res_dict):
    """对比 Python 结果和 C 语言结果，并输出差异报告。"""
    
    # 检查长度
    if len(py_res) != len(c_res_dict):
        print(f"\n⚠️ 警告: 结果长度不匹配! Python: {len(py_res)}, C Log: {len(c_res_dict)}")
    
    total_diff_count = 0
    print("\n" + "="*50)
    print(f"--- FFT 结果对比报告 (Max Tolerance: {MAX_ERROR_TOLERANCE}) ---")
    print("="*50)

    # 逐点对比
    for i in range(N):
        py_R, py_I = py_res[i]['R'], py_res[i]['I']
        
        if i in c_res_dict:
            c_R, c_I = c_res_dict[i]['R'], c_res_dict[i]['I']
        else:
            print(f"Point {i}: C Log 缺少该点数据。跳过。")
            continue

        # 计算绝对差值
        diff_R = abs(py_R - c_R)
        diff_I = abs(py_I - c_I)
        
        # 检查是否超出容差
        is_diff = diff_R > MAX_ERROR_TOLERANCE or diff_I > MAX_ERROR_TOLERANCE
        
        if is_diff:
            total_diff_count += 1
            print(f"\n❌ Point {i} - 发现差异!")
            print(f"    Python R/I: {py_R:.4f} / {py_I:.4f}")
            print(f"    C Log R/I:  {c_R:.4f} / {c_I:.4f}")
            print(f"    Diff R:     {diff_R:.4f} (Tol: {MAX_ERROR_TOLERANCE})")
            print(f"    Diff I:     {diff_I:.4f} (Tol: {MAX_ERROR_TOLERANCE})")
            
    print("\n" + "="*50)
    if total_diff_count == 0:
        print(f"🎉 验证成功！所有 {N} 个点的结果都在容差 ({MAX_ERROR_TOLERANCE}) 范围内。")
    else:
        print(f"❌ 验证失败！共发现 {total_diff_count} 个点的结果超出容差。")
    print("="*50)


# --- 主执行流程 ---
if __name__ == "__main__":
    
    # 1. 运行 Python FFT 并获取结果
    python_results_list = run_python_fft(rfft_test_input)

    # 2. 解析 C 语言日志结果
    # 注意：我们使用您提供的 LOG_REGEX 确保解析 Mag 值
    c_log_results_dict = parse_c_log(C_LOG_FILEPATH)
    
    # 3. 对比结果并生成报告 (文本报告)
    compare_results(python_results_list, c_log_results_dict)
    
    # 4. 绘图对比 (可视化报告)
    plot_comparison(python_results_list, c_log_results_dict)