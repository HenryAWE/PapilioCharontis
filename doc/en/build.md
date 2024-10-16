# Building and Installing
## Prerequisites
1. Compiler that supports C++20. Recommended compilers: `MSVC 19.3` (`Visual Studio 2022`) or higher, `GCC 12` or higher, `Clang 15` or higher.
2. `CMake`: `3.20` or higher.
3. `Ninja`

## Build the Project
Run `build.sh` from the project root directory. For Windows, run `build.ps1` instead.
```sh
./build.sh -DCMAKE_BUILD_TYPE=Release
```
```ps1
# Windows
./build.ps1 -DCMAKE_BUILD_TYPE=Release
```

## Install the Library
Run `cmake --install` in the `build/` directory.

## Use the Library in `CMakeLists.txt`
```cmake
find_package(Papilio REQUIRED)

target_link_libraries(main PRIVATE papilio::papilio)
```
