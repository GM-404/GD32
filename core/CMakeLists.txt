# 最小cmake版本要求
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# 项目名称
# 设置项目名称
project(mcu_cmake VERSION 1.0.0 LANGUAGES C CXX ASM)

# 设置语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------- CMSIS 库配置 -----------------
set(THIRDPARTY_ROOT_DIR ${CMAKE_PROJECT_ROOT}/ThirdParth)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)
include_directories(${CMAKE_PROJECT_ROOT}/configs/default)

# 添加 CMSIS 核心和 DSP 库的头文件路径
include_directories(
    ${THIRDPARTY_ROOT_DIR}/CMSIS/Include
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Include
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/privateInclude
)
# 查找 FFTW3 包 (由 vcpkg 提供)
# vcpkg 会自动配置好搜索路径
# 包名是 FFTW3 (大小写敏感)
# find_package(FFTW3 CONFIG REQUIRED)


add_executable(gd32_radar_demo
    Source/main.c 
    Source/radar_log.c
    Source/radar_dismantle.c
    Source/radar_window.c
    Source/radar_1dfft.c
    Source/radar_clutter_removal.c
    Source/radar_2dfft.c
    Source/radar_cfar.c
    Source/radar_dbf.c

    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/TransformFunctions/arm_cfft_init_f32.c
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/TransformFunctions/arm_cfft_f32.c
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/CommonTables/arm_common_tables.c
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/CommonTables/arm_const_structs.c
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/TransformFunctions/arm_bitreversal.c
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/TransformFunctions/arm_bitreversal2.c
    ${THIRDPARTY_ROOT_DIR}/ARMDSP/Source/TransformFunctions/arm_cfft_radix8_f32.c
    
)
target_compile_definitions(gd32_radar_demo PRIVATE 
    ARM_MATH_CM33
    ARM_MATH_DSP
    __FPU_USED
)

# target_include_directories(gd32_radar_demo PRIVATE 
#     "../ThirdParth/ARMDSP/Include"
#     "../ThirdParth/CMSIS/Core/Include"
# )

target_link_libraries(gd32_radar_demo
    -lm
    -specs=nosys.specs
)


# 链接 FFTW3 库到您的目标
# FFTW3::fftw3 是 vcpkg 提供的 CMake 目标
#target_link_libraries(gd32_radar_demo PRIVATE FFTW3::fftw3)
# 确保头文件目录被包含 (通常 find_package 会自动处理)
# target_include_directories(radar_fft PRIVATE ${FFTW3_INCLUDE_DIRS})
