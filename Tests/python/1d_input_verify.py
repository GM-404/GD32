import matplotlib.pyplot as plt
import re
import tkinter as tk
from tkinter import filedialog
import os

def parse_radar_log_content(log_content):
    """
    从日志内容字符串中解析雷达数据。

    Args:
        log_content (str): 包含雷达日志的字符串。

    Returns:
        dict: 一个字典，键是天线编号 (int)，值是一个包含 'points' 和 'parsed_values' 列表的字典。
            如果内容中没有找到有效数据，则返回空字典。
    """
    data_by_ant = {}

    # 正则表达式匹配每行数据
    # Group 1: Ant number
    # Group 2: Point number
    # Group 3: Parsed value
    pattern = re.compile(r"Ant (\d+) / Chirp \d+ / Point (\d+) \(Raw: \d+, Parsed: (-?\d+)\)")

    for line in log_content.splitlines():
        match = pattern.match(line)
        if match:
            ant_num = int(match.group(1))
            point = int(match.group(2))
            parsed_value = int(match.group(3))

            if ant_num not in data_by_ant:
                data_by_ant[ant_num] = {'points': [], 'parsed_values': []}

            data_by_ant[ant_num]['points'].append(point)
            data_by_ant[ant_num]['parsed_values'].append(parsed_value)
            
    return data_by_ant

def process_single_file(file_path, plot_data=True):
    """
    处理单个雷达日志文件：解析数据，并可选地绘制图表。

    Args:
        file_path (str): 雷达日志文件的路径。
        plot_data (bool): 如果为 True，则绘制幅值图；否则只解析数据。

    Returns:
        dict: 一个字典，键是天线编号 (int)，值是对应的 Parsed 值列表 (list of int)。
              如果文件不存在或没有找到有效数据，则返回空字典。
              如果 plot_data 为 True，这个字典的结构会更详细，包含 'points' 和 'parsed_values'。
    """
    try:
        with open(file_path, 'r') as f:
            log_content = f.read()
    except FileNotFoundError:
        print(f"Error: File not found at {file_path}")
        return {}
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return {}

    data_by_ant = parse_radar_log_content(log_content)

    if not data_by_ant:
        print(f"No valid radar data found in {file_path}.")
        return {}

    file_name = os.path.basename(file_path)

    if plot_data:
        # 绘制每个天线的图
        for ant_num, data in data_by_ant.items():
            plt.figure(figsize=(12, 6)) # 设置图的大小
            plt.plot(data['points'], data['parsed_values'], marker='o', linestyle='-')
            plt.title(f'Amplitude Plot for Antenna {ant_num} (File: {file_name})')
            plt.xlabel('Point Index')
            plt.ylabel('Parsed Amplitude Value')
            plt.grid(True)
            plt.tight_layout()
            plt.show() # 显示图表
    else:
        # 如果不绘图，我们可能只想返回 Parsed 值列表
        extracted_parsed_values = {ant_num: data['parsed_values'] for ant_num, data in data_by_ant.items()}
        print(f"Extracted data for {file_name}:")
        for ant_num, parsed_values in extracted_parsed_values.items():
            print(f"  Antenna {ant_num}: {parsed_values[:50]}... (Total {len(parsed_values)} points)") # 打印前10个值和总数
        return extracted_parsed_values
    
    return data_by_ant # 如果绘图，也返回解析后的数据 (包含 points 和 parsed_values)

def main():
    """
    主函数，处理文件选择和功能选择。
    """
    root = tk.Tk()
    root.withdraw() # 隐藏主窗口

    print("Please choose an action:")
    print("1. Extract data and plot charts")
    print("2. Only extract data (no plotting)")
    
    choice = input("Enter your choice (1 or 2): ")

    plot_option = False
    if choice == '1':
        plot_option = True
    elif choice == '2':
        plot_option = False
    else:
        print("Invalid choice. Exiting.")
        return

    file_paths = filedialog.askopenfilenames(
        title="Select Radar Log Files",
        filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
    )

    if not file_paths:
        print("No files selected.")
        return

    all_results = {}
    for file_path in file_paths:
        print(f"\n--- Processing file: {file_path} ---")
        result = process_single_file(file_path, plot_data=plot_option)
        if result:
            file_name = os.path.basename(file_path)
            all_results[file_name] = result

    if not plot_option and all_results:
        print("\n--- Summary of All Extracted Data (No Plotting) ---")
        for file_name, data in all_results.items():
            print(f"File: {file_name}")
            for ant_num, parsed_values in data.items():
                print(f"  Antenna {ant_num} has {len(parsed_values)} 'Parsed' values.")


if __name__ == "__main__":
    main()