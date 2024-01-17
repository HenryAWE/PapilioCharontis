# 常见问题（FAQ）
## 1、对 `format` 函数的调用出现歧义
这是 C++ 的[实参依赖查找（argument-dependent lookup, ADL）](https://zh.cppreference.com/w/cpp/language/adl)造成的，通常发生在 `using namespace papilio` 之后并使用 `format` 输出 `std` 命名空间下的类型时。  
使用 `papilio::format` 显式指定正确的函数即可。也可以使用 `PAPILIO_NS` 宏（需要包含头文件 `<papilio/macros.hpp>`）。
