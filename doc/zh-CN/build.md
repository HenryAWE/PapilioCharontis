# 构建
## 前置要求
1. 任意支持 C++20 的编译器，推荐：`MSVC 19.3`（`Visual Studio 2022`）或以上、`GCC 12` 或以上、`Clang 15` 或以上
2. `CMake`：`3.20` 或以上
3. `Ninja`

## 构建项目
在项目根目录运行 `build.sh`。在 Windows 上运行 `build.ps1` 作为替代。
```sh
./build.sh -DCMAKE_BUILD_TYPE=Release
```
```ps1
# Windows
./build.ps1 -DCMAKE_BUILD_TYPE=Release
```

## 安装库
在 `build/` 目录执行 `cmake --install .` 即可。
