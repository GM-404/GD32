#ifndef ADD_H
#define ADD_H

#include <stdlib.h> // 需要 malloc 和 free

// ------------------------------------
// 结构体声明 (模拟 C++ 类)
// ------------------------------------
typedef struct {
    int data; 
} ExampleClass;

// ------------------------------------
// 函数声明 (模拟 C++ 方法)
// ------------------------------------

// 模块级函数
int add(int a, int b);

// 构造函数 (创建对象并初始化)
ExampleClass* ExampleClass_create(int value);

// 析构函数 (释放对象内存)
void ExampleClass_destroy(ExampleClass* obj);

// Getter 方法
int ExampleClass_getData(const ExampleClass* obj);

// Setter 方法
void ExampleClass_setData(ExampleClass* obj, int value);

#endif /* ADD_H */