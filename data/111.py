# 导入 os 模块以检查文件存在性 (在 Canvas 环境中，我们假设文件可访问)
import os

# 定义要处理的文件名
FILE_NAME = "111.txt"

# ------------------------------------------------------
# 1. 从上传的文档中读取数据
# ------------------------------------------------------
try:
    # 假设在 Canvas 环境中，可以直接通过文件名访问上传的文本文件
    # 注意：这里我们读取了整个文件内容，并假设它是空格分隔的十六进制字节
    # 如果文件路径需要调整，请根据实际环境修改
    with open(FILE_NAME, 'r') as f:
        file_content = f.read()

except FileNotFoundError:
    print(f"错误: 找不到文件 {FILE_NAME}。请确保文件已上传并路径正确。")
    # 如果文件找不到，使用一个示例数据作为回退，方便测试逻辑
    file_content = "BB 55 AA 55 01 00 00 00 01 00 01 00 00 02 AD DE 80 00 40 00 00 40 00 00 F3 05 ED DC DF E7 EB E9"
    print("使用硬编码的示例数据继续处理。")
except Exception as e:
    print(f"读取文件 {FILE_NAME} 时发生错误: {e}")
    file_content = ""


# ------------------------------------------------------
# 2. 清理输入并提取有效的十六进制数字字符串
# ------------------------------------------------------
# 移除所有空白字符（包括空格、换行符等），然后按空格分割。
# 这样可以处理像 "BB 55 AA" 这种空格分隔格式
cleaned_data = []
# 使用 split() 默认按空格、tab、换行符等分割，并自动忽略多个连续的空格
for item in file_content.split():
    # 确保 item 是一个有效的十六进制字符串（例如，长度为 2 的字节）
    hex_byte = item.strip().upper()
    
    # 过滤掉空的字符串，并且如果需要更严格的检查，可以添加长度和字符集检查
    if hex_byte and len(hex_byte) <= 2:
        cleaned_data.append(hex_byte)

# 定义每行显示的字节数
BYTES_PER_LINE = 16

# ------------------------------------------------------
# 3. 格式化输出
# ------------------------------------------------------
formatted_output = []
for i in range(0, len(cleaned_data), BYTES_PER_LINE):
    # 获取当前行的切片
    line_slice = cleaned_data[i:i + BYTES_PER_LINE]
    
    # 在每个字节前加上 "0x" 前缀
    prefixed_line = [f"0x{byte}" for byte in line_slice]
    
    # 将行中的字节用 ", " 连接起来
    formatted_line = ", ".join(prefixed_line)
    
    formatted_output.append(formatted_line)

# 4. 统计个数
total_count = len(cleaned_data)

# 5. 打印结果
print("------------------------------------------------------")
#print(f"格式化后的十六进制数组 (来自 {FILE_NAME}):")
print("------------------------------------------------------")
# 打印格式化后的数据，每行末尾加上逗号
for line in formatted_output:
    # 确保只有非空行才打印逗号
    if line:
        print(line + ",")

print("\n------------------------------------------------------")
print(f"✅ 总共统计到 {total_count} 个十六进制数字（字节）。")
print("------------------------------------------------------")

# 转换为一个 C 语言风格的数组字符串，方便复制粘贴
c_array_str = "{" + ",\n ".join(formatted_output) + "}"
print("\n--- C/C++ 数组格式 (方便复制粘贴) ---")
#print(c_array_str)

# ------------------------------------------------------
# 6. 将结果保存到文件中
# ------------------------------------------------------
OUTPUT_FILE = "output.txt"
try:
    with open("output.txt", "w", encoding="utf-8") as f:
        f.write("------------------------------------------------------\n")
        f.write(f"格式化后的十六进制数组 (来自 {FILE_NAME}):\n")
        f.write("------------------------------------------------------\n")
        for line in formatted_output:
            if line:
                f.write(line + ",\n")
        f.write("\n------------------------------------------------------\n")
        f.write(f"✅ 总共统计到 {total_count} 个十六进制数字（字节）。\n")
        f.write("------------------------------------------------------\n")
        f.write("\n--- C/C++ 数组格式 ---\n")
        f.write(c_array_str + "\n")
    print(f"\n✅ 结果已成功保存到文件: {OUTPUT_FILE}")
except Exception as e:
    print(f"保存文件 {OUTPUT_FILE} 时发生错误: {e}")