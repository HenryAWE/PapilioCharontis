# 构建
## 前置要求
- `CMake`：`3.20` 或以上
- 支持 C++20 的编译器  
  推荐的编译器： `MSVC 19.3`（`Visual Studio 2022`）或以上、`GCC 12` 或以上
- `GoogleTest` （仅当需要构建单元测试时）

## 构建项目
### 配置
```bash
mkdir build
cd build
cmake ..
```
如果需要构建单元测试，请添加参数 `-Dpapilio_build_unit_test=1`
### 编译
```bash
cmake --build .
```
### 运行单元测试
```bash
ctest .
```
