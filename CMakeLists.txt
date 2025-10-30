
# 最小cmake版本要求
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

#INCLUDE(cmake/arm-none-eabi-gcc.cmake)
unset(CMAKE_TOOLCHAIN_FILE)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/arm-none-eabi-gcc.cmake CACHE STRING "")

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


# 设置项目根目录，后续如果有需要在其他地方引用项目根目录，可以使用这个变量
set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Project root directory set to: ${CMAKE_PROJECT_ROOT}")

# 1. 定义 CMSIS-DSP 库的路径
set(CMSIS_ROOT_DIR ${CMAKE_SOURCE_DIR}/ThirdParth/CMSIS)
set(CMSIS_DSP_ROOT_DIR ${CMSIS_ROOT_DIR}/DSP)

# 指定可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# 设置构建类型的默认值为 Debug，可根据需要修改
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

# 包含子目录
add_subdirectory(configs/default)
add_subdirectory(core)
# 执行 Test 目录下的 CMakeLists.txt，用于配置单元测试相关设置
add_subdirectory(Tests)
# 执行 PythonWrapper 目录下的 CMakeLists.txt，用于配置 Python 包相关设置
add_subdirectory(PythonWrapper)

