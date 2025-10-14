#include "add.h"
#include <stdio.h> // 仅用于可能的调试输出

// ------------------------------------
// 模块级函数实现
// ------------------------------------
int add(int a, int b)
{
    return a + b;
}

// ------------------------------------
// 结构体方法实现
// ------------------------------------

// 构造函数：分配内存并初始化
ExampleClass* ExampleClass_create(int value)
{
    // 分配内存
    ExampleClass* obj = (ExampleClass*)malloc(sizeof(ExampleClass));
    
    if (obj != NULL) {
        // 初始化数据
        obj->data = value;
    }
    return obj;
}

// 析构函数：释放内存
void ExampleClass_destroy(ExampleClass* obj)
{
    if (obj != NULL) {
        free(obj);
        // 在 C 中，通常建议将外部指针设置为 NULL，但这里只负责内部释放
    }
}

// Getter 方法
int ExampleClass_getData(const ExampleClass* obj)
{
    if (obj == NULL) {
        // 处理空指针情况
        return -1; 
    }
    return obj->data;
}

// Setter 方法
void ExampleClass_setData(ExampleClass* obj, int value)
{
    if (obj != NULL) {
        obj->data = value;
    }
}