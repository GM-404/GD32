import json
import os
import sys

# --- 配置参数 ---
INPUT_FILENAME = "TEST2.json"
TARGET_FRAME_ID = 3  # <--- 设定您想要提取的帧ID，例如 0, 1, 2, ...
OUTPUT_FILENAME_FORMAT = "frame_{}_data.json" # 输出文件名格式

def extract_and_save_frame(input_file, target_id, output_format):
    """
    从 JSON 列表中提取具有指定 frame_id 的帧数据并保存到新文件。

    :param input_file: 输入 JSON 文件名 (e.g., "TEST2.json")
    :param target_id: 目标帧ID (int)
    :param output_format: 输出文件名格式 (e.g., "frame_{}_data.json")
    :return: None
    """
    if not os.path.exists(input_file):
        print(f"错误: 输入文件 '{input_file}' 不存在。")
        return

    try:
        # 1. 加载原始 JSON 文件
        with open(input_file, 'r') as f:
            data = json.load(f)
    except json.JSONDecodeError:
        print(f"错误: 文件 '{input_file}' 不是一个有效的 JSON 文件。")
        return
    except Exception as e:
        print(f"读取文件时发生错误: {e}")
        return

    # 2. 遍历数据，查找目标帧
    target_frame = None
    
    # 假设 JSON 是一个帧对象的列表，例如：
    # [{"frame_id": 0, "frame_data": [...]}, {"frame_id": 1, "frame_data": [...]}, ...]
    if isinstance(data, list):
        for frame in data:
            if isinstance(frame, dict) and frame.get('frame_id') == target_id:
                target_frame = frame
                break # 找到第一个匹配的帧后停止
    else:
        print(f"警告: 根对象不是列表。尝试直接检查根对象...")
        if isinstance(data, dict) and data.get('frame_id') == target_id:
             target_frame = data

    if target_frame:
        # 3. 保存找到的帧数据到一个新的 JSON 文件中
        output_filename = output_format.format(target_id)
        
        # 将单个帧对象放入一个列表中，以保持原始文件的格式一致性（如果它是一个帧列表）
        output_data = [target_frame]
        
        try:
            with open(output_filename, 'w') as f:
                # 使用 indent=4 使输出文件更易读
                json.dump(output_data, f, indent=4) 
            print(f"✅ 成功提取 frame_id {target_id} 的数据。")
            print(f"数据已保存至: {output_filename}")
        except Exception as e:
            print(f"保存文件时发生错误: {e}")
    else:
        print(f"❌ 未找到 frame_id 为 {target_id} 的数据帧。")

if __name__ == "__main__":
    # 可以通过命令行参数覆盖默认的帧ID
    if len(sys.argv) > 1:
        try:
            cmd_target_id = int(sys.argv[1])
            extract_and_save_frame(INPUT_FILENAME, cmd_target_id, OUTPUT_FILENAME_FORMAT)
        except ValueError:
            print(f"错误: 命令行参数 '{sys.argv[1]}' 必须是一个整数。")
            extract_and_save_frame(INPUT_FILENAME, TARGET_FRAME_ID, OUTPUT_FILENAME_FORMAT)
    else:
        extract_and_save_frame(INPUT_FILENAME, TARGET_FRAME_ID, OUTPUT_FILENAME_FORMAT)