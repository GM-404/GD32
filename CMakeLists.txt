
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

# 设置项目根目录，后续如果有需要在其他地方引用项目根目录，可以使用这个变量
set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Project root directory set to: ${CMAKE_PROJECT_ROOT}")

# 指定可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})



# # 检查编译器是否支持 LTO链接时间优化是一种编译器优化技术，它允许编译器在链接阶段对整个程序进行全局分析和优化，而不仅仅是在编译单个源文件时进行局部优化。
# 通过 LTO，编译器可以在链接时进行更深入的代码分析和转换，例如内联函数调用、消除冗余代码等，从而生成性能更优的可执行文件。
# if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
# endif()

# 设置构建类型的默认值为 Debug，可根据需要修改
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

# 包含子目录
add_subdirectory(configs/default)
add_subdirectory(core)


