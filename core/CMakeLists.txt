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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)

include_directories(${CMAKE_PROJECT_ROOT}/configs/default)

# 查找 FFTW3 包 (由 vcpkg 提供)
# vcpkg 会自动配置好搜索路径
# 包名是 FFTW3 (大小写敏感)
find_package(FFTW3 CONFIG REQUIRED)
# 添加您的可执行文件
# add_executable(radar_fft_test Source/test.c)

add_executable(lesson1_1 
            Source/main.c 
            Source/radar_log.c
            Source/radar_dismantle.c
            Source/radar_1dfft.c
            Source/radar_2dfft.c
            Source/radar_cfar.c
        )


# 链接 FFTW3 库到您的目标
# FFTW3::fftw3 是 vcpkg 提供的 CMake 目标
# target_link_libraries(radar_fft_test PRIVATE FFTW3::fftw3)
target_link_libraries(lesson1_1 PRIVATE FFTW3::fftw3)

# 确保头文件目录被包含 (通常 find_package 会自动处理)
# target_include_directories(radar_fft PRIVATE ${FFTW3_INCLUDE_DIRS})
