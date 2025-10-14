#include <stdio.h>
#include "add.h" // 包含我们自己的头文件
#include <math.h> // 包含标准数学库

int main()
{
    printf("--- 验证 CMake C 语言配置 ---\n");

    // 1. 测试 add 函数 (模块级函数)
    int sum_result = add(10, 20);
    printf("Result of add(10, 20) is: %d\n", sum_result);

    // 2. 测试数学函数 (标准库)
    double sqrt_result = sqrt(16.0);
    printf("Square root of 16 is: %f\n", sqrt_result);

    // 3. 测试 ExampleClass 结构体 (模拟对象)
    printf("--- 结构体测试 ---\n");
    
    // 创建对象 (调用构造函数)
    ExampleClass* myObject = ExampleClass_create(100);

    if (myObject != NULL) {
        // 获取数据
        printf("Initial data: %d\n", ExampleClass_getData(myObject));
        
        // 设置数据
        ExampleClass_setData(myObject, 250);
        printf("New data: %d\n", ExampleClass_getData(myObject));

        // 销毁对象 (调用析构函数)
        ExampleClass_destroy(myObject);
    } else {
        fprintf(stderr, "Error: Failed to create ExampleClass object.\n");
        return 1;
    }

    printf("--- 验证成功 ---\n");
    return 0;
}