include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(test_frame_dismantle 
            src/frame_dismantle.c
            test_frame_data_dismantle.c
        )