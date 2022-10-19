# Building
## Prerequisites
- `CMake`: `3.20` or higher
- Compiler that supports C++20  
  Recommended compilers:  `MSVC 19.3` (`Visual Studio 2022`) or higher, `GCC 12` or higher
- `GoogleTest` (only when building unit tests is required)

## Build the Project
### Configure
```bash
mkdir build
cd build
cmake ..
```
If building unit tests is required, please add the argument `-Dpapilio_build_unit_test=1`
### Compile
```bash
cmake --build .
```
### Run Unit Tests
```bash
ctest .
```
