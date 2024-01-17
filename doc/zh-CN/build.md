# 构建
## 前置要求
1. 任意支持 C++20 的编译器，推荐：`MSVC 19.3`（`Visual Studio 2022`）或以上、`GCC 12` 或以上
2. `CMake`：`3.20` 或以上
3. `GoogleTest` （可选）：仅构建单元测试时需要

## 构建项目
### 配置
```bash
mkdir build
cd build
cmake ..
```
如果需要构建示例或单元测试，请添加参数 `-Dpapilio_build_example=1` 或 `-Dpapilio_build_unit_test=1`。  
更多编译选项的文档在[自定义构建](custom_build.md)中。

### 编译
```bash
cmake --build .
```
### 运行单元测试
```bash
ctest .
```
