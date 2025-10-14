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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(lesson1_1 
            Source/main.c 
            Source/add.c)

# # 生成对象 - 添加需要的文件
# add_library(Algo OBJECT
#     radar_algo.c
# )

# # 包含路径
# target_include_directories(Algo PUBLIC
#     ${ROOT_DIR}/Alg
# )

# # 链接CMSIS库
# target_link_libraries(Algo PUBLIC
#     BSP
#     ARMDSP
# )
