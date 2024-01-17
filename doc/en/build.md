# Building
## Prerequisites
1. Compiler that supports C++20. Recommended compilers: `MSVC 19.3` (`Visual Studio 2022`) or higher, `GCC 12` or higher.
2. `CMake`: `3.20` or higher.
3. `GoogleTest` (optional): Only required when building unit tests.

## Build the Project
### Configure
```bash
mkdir build
cd build
cmake ..
```
If you need to build examples or unit tests, add the parameter `-Dpapilio_build_example=1` or `-Dpapilio_build_unit_test=1`.  
Document for more build options are in the [Custom Build](custom_build.md).
### Compile
```bash
cmake --build .
```
### Run Unit Tests
```bash
ctest .
```
