# 项目名称及支持的语言
project(ARMDSP LANGUAGES C CXX ASM)

# 生成对象 - 添加需要的文件
add_library(ARMDSP OBJECT
    "${MIDDLEWARE_DIR}/ARMDSP/ComputeLibrary/Source/arm_cl_tables.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/BasicMathFunctions/BasicMathFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/BasicMathFunctions/BasicMathFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/BayesFunctions/BayesFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/BayesFunctions/BayesFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/CommonTables/CommonTables.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/CommonTables/CommonTablesF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/ComplexMathFunctions/ComplexMathFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/ControllerFunctions/ControllerFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/DistanceFunctions/DistanceFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/DistanceFunctions/DistanceFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/FastMathFunctions/FastMathFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/FastMathFunctions/FastMathFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/FilteringFunctions/FilteringFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/FilteringFunctions/FilteringFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/InterpolationFunctions/InterpolationFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/InterpolationFunctions/InterpolationFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/MatrixFunctions/MatrixFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/MatrixFunctions/MatrixFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/QuaternionMathFunctions/QuaternionMathFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/StatisticsFunctions/StatisticsFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/StatisticsFunctions/StatisticsFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/SupportFunctions/SupportFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/SupportFunctions/SupportFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/SVMFunctions/SVMFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/SVMFunctions/SVMFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/TransformFunctions/TransformFunctions.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/TransformFunctions/TransformFunctionsF16.c"
    "${MIDDLEWARE_DIR}/ARMDSP/Source/WindowFunctions/WindowFunctions.c"
)

# 包含路径
target_include_directories(ARMDSP PUBLIC
    "${MIDDLEWARE_DIR}/ARMDSP/Include"
    "${MIDDLEWARE_DIR}/ARMDSP/PrivateInclude"
    "${MIDDLEWARE_DIR}/ARMDSP/ComputeLibrary/Include"
)

# 链接CMSIS库
target_link_libraries(ARMDSP PUBLIC
    CMSIS
)
