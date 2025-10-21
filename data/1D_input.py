import json
import numpy as np
import os

# -------------------------------------------------------------
# 1. 雷达参数定义
# -------------------------------------------------------------
RADAR_ANT_COUNT = 2      # 采样天线数
RADAR_CHIRP_COUNT = 64   # 采样chirp数
RADAR_CHIRP_POINTS = 128 # 采样点数据 (RFFT_POINTS_1D)
TOTAL_DATA_BYTES = RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS # 16384

# -------------------------------------------------------------
# 2. 模拟 C 语言的 frame_data_dismantle 函数 (重排和类型转换)
# -------------------------------------------------------------
def frame_data_dismantle_py(raw_data_bytes):
    """
    模拟 C 语言中的 frame_data_dismantle 函数，将一维 uint8_t 字节流
    解析并重排为 (Ant x Chirp x Point) 的 int8_t 三维数组。
    """
    if len(raw_data_bytes) != TOTAL_DATA_BYTES:
        raise ValueError(f"数据字节数不匹配。预期: {TOTAL_DATA_BYTES}, 实际: {len(raw_data_bytes)}")

    # 目标三维数组 (Ant x Chirp x Point)
    parsed_radar_data = np.zeros(
        (RADAR_ANT_COUNT, RADAR_CHIRP_COUNT, RADAR_CHIRP_POINTS),
        dtype=np.int8
    )

    current_index = 0
    
    # C 语言的重排逻辑
    for ant in range(RADAR_ANT_COUNT):
        for chirp in range(RADAR_CHIRP_COUNT):
            for point in range(RADAR_CHIRP_POINTS):
                
                raw_value = raw_data_bytes[current_index]
                
                # 模拟 C 语言的 (int8_t)raw_value 转换 (uint8_t -> int8_t)
                signed_value = raw_value if raw_value <= 127 else raw_value - 256

                parsed_radar_data[ant, chirp, point] = signed_value
                
                current_index += 1

    return parsed_radar_data

# -------------------------------------------------------------
# 3. 运行逻辑：提取 FFT 输入数据并导出
# -------------------------------------------------------------
OUTPUT_FILENAME = 'rfft_input_buffer.txt'

# 设定要提取的 Chirp 数据
TARGET_ANT = 0
TARGET_CHIRP = 0

# 1. 读取 JSON 文件
try:
    with open('TEST2.json', 'r') as f:
        data = json.load(f)
except Exception as e:
    print(f"错误: 无法读取或解析 TEST2.json: {e}")
    exit()

# 使用第一帧数据
frame_info = data[0]
raw_payload = frame_info.get("frame_data", [])
frame_index = frame_info.get("frame_id", 0)

if len(raw_payload) != TOTAL_DATA_BYTES:
    print(f"错误: JSON 数据长度不匹配。预期 {TOTAL_DATA_BYTES}, 实际 {len(raw_payload)}")
    exit()

# 2. 执行数据重排
try:
    parsed_data_int8 = frame_data_dismantle_py(raw_payload)
except ValueError as e:
    print(f"❌ 数据重排失败: {e}")
    exit()

# 3. 提取目标 Chirp 的数据并转换为 float32_t (RFFT 输入)
# 这模拟了 radar_fft.c 中 int8_t 到 float32_t 的显式转换：
# rfft_input_buffer[point] = (float32_t)raw_data[ant][chirp][point];
rfft_input_int8 = parsed_data_int8[TARGET_ANT, TARGET_CHIRP, :]
rfft_input_buffer_float32 = rfft_input_int8.astype(np.float32)

# 4. 构造 C 数组输出字符串
c_array_lines = []
for i in range(0, len(rfft_input_buffer_float32), 8): # 每行 8 个 float
    chunk = rfft_input_buffer_float32[i:i + 8]
    # 使用 %.4ff 格式，保证精度，并添加 'f' 后缀
    float_values = [f"{f:.4f}" for f in chunk]
    c_array_lines.append(f"    {', '.join(float_values)},")

c_array_output = f"""
// ----------------------------------------------------------------------------------
// C 数组定义：1D FFT (RFFT) 的输入数据
// 来源：TEST2.json, 帧索引 {frame_index}, 天线 {TARGET_ANT}, Chirp {TARGET_CHIRP}
// 总点数: {RADAR_CHIRP_POINTS} (float32_t)
// ----------------------------------------------------------------------------------
#define RFFT_INPUT_POINTS {RADAR_CHIRP_POINTS}
const float32_t rfft_test_input[RFFT_INPUT_POINTS] = {{
{'\n'.join(c_array_lines)}
}};
"""

# 5. 保存到文件
try:
    with open(OUTPUT_FILENAME, 'w') as f:
        f.write(c_array_output)
    
    print("-------------------------------------------------------")
    print(f"✅ FFT 输入数据准备完成。")
    print(f"来源: Ant {TARGET_ANT}, Chirp {TARGET_CHIRP}")
    print(f"💾 C 数组定义已保存到文件: {os.path.abspath(OUTPUT_FILENAME)}")
    print("-------------------------------------------------------")

except Exception as e:
    print(f"❌ 写入文件失败: {e}")