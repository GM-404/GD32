# 项目名称及支持语言
project(CMSIS LANGUAGES C CXX ASM)

# 生成接口
add_library(CMSIS INTERFACE)

# 包含路径
target_include_directories(CMSIS INTERFACE
    "${CORE_DIR}"
    "${DRIVER_DIR}/CMSIS/Include"
    "${DRIVER_DIR}/CMSIS/Include/m-profile"
    "${DRIVER_DIR}/CMSIS/Device/GD/GD32W51x/Include"
)
