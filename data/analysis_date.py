import json
import struct
import numpy as np
import base64
import os

# -------------------------------------------------------------
# C 结构体及参数定义
# -------------------------------------------------------------
RADAR_ANT_COUNT = 2      # 采样天线数
RADAR_CHIRP_COUNT = 64   # 采样chirp数
RADAR_CHIRP_POINTS = 128 # 采样点数据
TOTAL_DATA_BYTES = RADAR_ANT_COUNT * RADAR_CHIRP_COUNT * RADAR_CHIRP_POINTS # 16384 bytes

FRAME_HEAD = 0x55AA55BB
FRAME_TAIL = 0x55CC55DD
FRAME_INDEX = 0 # 示例帧索引
DATA_TYPE = 0   # int8_t 双极性ADC

# C 结构体头部格式 (24 字节): 
# I: uint32 (x2), H: uint16 (x4), B: uint8 (x2), H: uint16 (x2), I: uint32 (x1)
# 目标平台使用小端序 (<) 且默认对齐 (头部共 24 字节)
HEADER_FORMAT = '<IIHHBBHHHI' 
HEADER_SIZE = 24 # struct.calcsize(HEADER_FORMAT)
TAIL_FORMAT = '<I'
TAIL_SIZE = 4

def pack_radar_frame(raw_payload, frame_index):
    """
    将原始 int8_t 数据（以 uint8_t 列表形式）封装为 sample_frame_t 结构体的字节流。
    """
    
    # 构造头部数据
    header_data = [
        FRAME_HEAD,         # uint32_t frame_head
        frame_index,        # uint32_t frame_index
        1,                  # uint16_t curr_pack (单包)
        1,                  # uint16_t total_pack (单包)
        DATA_TYPE,          # uint8_t data_type
        RADAR_ANT_COUNT,    # uint8_t samp_ants
        0xDEAD,             # uint16_t data_crc16 (Placeholder)
        RADAR_CHIRP_POINTS, # uint16_t samp_points
        RADAR_CHIRP_COUNT,  # uint16_t samp_chirps
        TOTAL_DATA_BYTES    # uint32_t data_bytes
    ]
    
    # 1. 封装头部
    packed_header = struct.pack(HEADER_FORMAT, *header_data)
    
    # 2. 封装数据部分
    packed_data = bytes(raw_payload)
    
    # 3. 封装帧尾
    packed_tail = struct.pack(TAIL_FORMAT, FRAME_TAIL)
    
    # 完整帧
    return packed_header + packed_data + packed_tail

# -------------------------------------------------------------
# 运行封装
# -------------------------------------------------------------
OUTPUT_FILENAME = 'packed_frame_data.txt'

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

# 2. 封装数据帧
packed_frame_bytes = pack_radar_frame(raw_payload, frame_index)

# 3. 构造 C 数组输出字符串
c_array_lines = []
for i in range(0, len(packed_frame_bytes), 16):
    chunk = packed_frame_bytes[i:i + 16]
    hex_values = [f"0x{b:02X}" for b in chunk]
    c_array_lines.append(f"    {', '.join(hex_values)},")

c_array_output = f"""
// ----------------------------------------------------------------------------------
// C 数组定义：由 Python 脚本生成
// 总字节数: {len(packed_frame_bytes)}
// 帧索引: {frame_index}
// ----------------------------------------------------------------------------------
#define PACKED_FRAME_SIZE {len(packed_frame_bytes)}
const uint8_t packed_frame_data[PACKED_FRAME_SIZE] = {{
{'\n'.join(c_array_lines)}
}};
"""

# 4. 保存到文件
try:
    with open(OUTPUT_FILENAME, 'w') as f:
        f.write(c_array_output)
    
    print("-------------------------------------------------------")
    print(f"✅ Python 封装完成。总字节数: {len(packed_frame_bytes)}")
    print(f"💾 C 数组定义已保存到文件: {os.path.abspath(OUTPUT_FILENAME)}")
    print("-------------------------------------------------------")
    print("请将 'packed_frame_data.txt' 中的内容复制到您的 C 工程 'main.c' 文件中。")

except Exception as e:
    print(f"❌ 写入文件失败: {e}")