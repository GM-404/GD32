#include <stdio.h>
#include <fftw3.h> // vcpkg 会自动处理此头文件的路径

int main() {
    int N = 8; // 采样点数
    fftw_complex *in, *out;
    fftw_plan p;

    // 1. 分配内存
    // 推荐使用 fftw_malloc 以保证内存对齐，提高 SIMD 性能
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    if (in == NULL || out == NULL) {
        printf("Error: fftw_malloc failed.\n");
        return 1;
    }

    // 2. 准备输入数据 (例如一个简单的脉冲)
    for (int i = 0; i < N; i++) {
        if (i == 0) {
            in[i][0] = 1.0; // 实部
            in[i][1] = 0.0; // 虚部
        } else {
            in[i][0] = 0.0;
            in[i][1] = 0.0;
        }
    }

    // 3. 创建 FFT "计划" (Plan)
    // 这是 FFTW 的核心：它会测试找到计算特定大小 FFT 的最快算法
    // FFTW_FORWARD: 正向变换
    // FFTW_ESTIMATE: 快速创建计划，如果追求极致性能可用 FFTW_MEASURE
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // 4. 执行 FFT
    fftw_execute(p); 

    // 5. 打印结果
    // 一个在 0 处的脉冲，其傅里叶变换在所有频率上都为 1
    printf("FFT 结果 (N=%d):\n", N);
    for (int i = 0; i < N; i++) {
        printf("out[%d] = { real: %f, imag: %f }\n", i, out[i][0], out[i][1]);
    }

    // 6. 清理资源
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return 0;
}