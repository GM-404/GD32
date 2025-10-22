# 雷达项目工程

## 目录说明

- 📂 `configs` 总体配置
- 📂 `code` 工程代码
- 📂 `data` 数据
- 📂 `doc` 文档
- 📂 `PythonWrapper`用于绑定成python包以作验证
- 📂 `Tests` 测试
- 📂 `ThirdParth` 第三方库

---
## 安装FFTW库
#### 步骤一：安装 vcpkg
1. 下载 vcpkg：
打开一个终端 (PowerShell 或 CMD)，克隆 vcpkg 仓库。推荐将其放在一个固定的位置，例如 C:\dev\vcpkg 或 %USERPROFILE%\vcpkg。
```
git clone https://github.com/microsoft/vcpkg
```
2. 引导 (Bootstrap) vcpkg：
进入 vcpkg 目录并运行引导脚本。
```
cd vcpkg
.\bootstrap-vcpkg.bat
```
#### 步骤二：为项目安装 FFTW3
vcpkg 有两种模式：经典模式（全局安装）和清单模式（项目局部安装）。推荐使用清单模式，因为它更干净，可以随项目一起分发。

1. 创建项目文件夹：
为雷达项目创建一个新文件夹，并用 VSCode 打开它。例如：Radar_FFT_Project。

2. 创建 vcpkg 清单文件：
在您的项目根目录 ( Radar_FFT_Project ) 下创建一个名为 vcpkg.json 的文件。

3. 编辑 vcpkg.json：
将 FFTW3 添加为依赖项。文件内容如下：
```JSON
{
  "name": "radar-fft-project",
  "version-string": "1.0.0",
  "dependencies": [
    "fftw3"
  ]
}
```
>说明： fftw3 是 vcpkg 中 FFTW 库的包名。

#### 步骤三：配置 CMake 与 VSCode
这是最关键的一步：让 CMake 知道如何通过 vcpkg 找到 FFTW。

创建 CMakeLists.txt 文件：
在项目根目录中创建 CMakeLists.txt 文件。

配置 VSCode 的 CMake 插件：
当 VSCode 的 CMake Tools 插件第一次询问如何配置项目时（或者您可以按 Ctrl+Shift+P 并运行 CMake: Configure），它会询问 "CMakeLists.txt" 和 "Kit" (编译器)。
最重要的是，您需要告诉 CMake 使用 vcpkg 的工具链文件 (toolchain file)。
打开 VSCode 设置 ( Ctrl+, )。
搜索 cmake.configureArgs。
点击 "在 settings.json 中编辑"。
添加以下配置，请务必将路径替换为自己的 vcpkg 路径：
.vscode/settings.json
```JSON
{
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
    ]
}
```
重新配置项目：
保存 settings.json 后，VSCode 底部状态栏可能会提示 "CMake configuration needs to update"。点击 "Yes" 或手动运行 CMake: Configure。

此时，vcpkg 将自动下载并编译 fftw3 及其依赖项。您将在 VSCode 的 "Output" 窗口中看到 vcpkg 的工作日志。
完成后，您就可以在 CMakeLists.txt 中使用 find_package(FFTW3 REQUIRED) 来链接 FFTW 库了。