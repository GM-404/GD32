# 设置工具链路径
set(TOOLCHAIN_DIR "D:\\SDK\\14.2rel1\\bin")

# 设置编译器标志
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS_RELEASE "")

# 是否启用色彩构建系统消息
set(CMAKE_COLOR_DIAGNOSTICS ON)

# 是否导出编译指令信息
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# 如果是win32则工具链后缀为.exe
if(WIN32)
    set(TOOLCHAIN_SUFFIX ".exe")
endif()

# 设置系统名称和处理器
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# 设置工具链前缀
set(TOOLCHAIN_PREFIX                "arm-none-eabi-")
if(DEFINED TOOLCHAIN_DIR)
    set(TOOLCHAIN_PREFIX            "${TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}")
endif()

# 设置编译标志
set(CPU_FLAGS                       "-mcpu=cortex-m33 -mfloat-abi=hard")
set(COMMON_FLAGS                    "${CPU_FLAGS} -std=gnu11 -fstack-usage -fdata-sections -ffunction-sections -fmessage-length=0 -fsigned-char -mthumb -Wall -Wno-missing-braces -Wno-format -Wno-strict-aliasing -Wno-unused-function -Wno-restrict")
# set(LINKER_FLAGS                    "${CPU_FLAGS} -Wl,--gc-sections -specs=nosys.specs -specs=nano.specs")
set(LINKER_FLAGS                    "${CPU_FLAGS} -Wl,--gc-sections")
set(ASM_FLAGS                       "-x assembler-with-cpp ${CPU_FLAGS}")

# 设置编译器
set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_SUFFIX})
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy${TOOLCHAIN_SUFFIX})
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJDUMP                   ${TOOLCHAIN_PREFIX}objdump${TOOLCHAIN_SUFFIX})
set(CMAKE_AS                        ${TOOLCHAIN_PREFIX}as${TOOLCHAIN_SUFFIX})
set(CMAKE_LD                        ${TOOLCHAIN_PREFIX}ld${TOOLCHAIN_SUFFIX})

# 设置编译器标志变量
set(CMAKE_C_FLAGS                   "${COMMON_FLAGS}" CACHE STRING "C Compiler Flags")
set(CMAKE_CXX_FLAGS                 "${COMMON_FLAGS} ${CPP_FLAGS}" CACHE STRING "C++ Compiler Flags")
set(CMAKE_ASM_FLAGS                 "${ASM_FLAGS}" CACHE STRING "ASM Compiler Flags")
set(CMAKE_EXE_LINKER_FLAGS          "${LINKER_FLAGS}" CACHE STRING "Linker Flags")

# 设置生成可执行文件后缀名
set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

# 静态链接库
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 设置汇编文件扩展名
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s;S;asm)

# 确保汇编文件使用正确的标志
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}" CACHE STRING "ASM Compiler Flags")
