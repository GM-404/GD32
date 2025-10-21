include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 测试文件生成
add_executable(test_frame_dismantle 
            src/frame_dismantle.c
            test_frame_data_dismantle.c
        )
add_executable(test_1dfft 
            src/radar_fft.c
            test_fft.c
)


