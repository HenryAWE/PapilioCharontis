**English** [简体中文](README.zh-CN.md)
# Papilio Charontis
A flexible formatting library for C++

## Overview
### Main Feature: Scripting
Using embedded script to control the output
```c++
std::string_view fmt =
    "There "
    "[if $0 != 1: 'are' else: 'is']"
    " {} "
    "apple[if $0 != 1: 's']";
format(fmt, 1); // Returns "There is 1 apple"
format(fmt, 2); // Returns "There are 2 apples"
```

### Accessing Member
Support indexing (integer or string), slicing and accessing member attributes
```c++
format("length of \"{0}\" is {0.length}", "hello");
// Returns "length of "hello" is 5"
format("{[:5]:}", "hello world"); // Returns "hello"
format("{[-5:]:}", "hello world"); // Returns "world"
format("{[0]:}", "hello world"); // Returns "h"
format("{[-1]:}", "hello world"); // Returns "d"
```

### Similar Functionalities to `<format>` in C++ 20
```c++
format("{}", 10); // Returns "10"
format("#{:08x}", 0xff); // Returns "#000000ff"
format("{1} and {0}", "second", "first"); // Returns "first and second"
using namespace papilio::literals;
format("{text} and {0}", "world", "text"_a = "hello"); // Returns "hello and world"
// You can use arg("text", "hello") instead if you don't want using namespace
```

### Unicode Support
You can conveniently use Unicode string in formatting function
```c++
format("{[:2]}", "你好，世界");
// Returns "你好" instead of first two bytes which cannot represent a valid character
format("Length: {0.length}; Size: {0.size}", "你好，世界");
// Returns "Length: 5; Size: 15"
```
NOTE: In order to run the above code, you need to make sure you code is saved in UTF-8 encoding and correct compiler flags is set. You can use the "u8" prefix to force the string to use UTF-8 encoding

[Quick Start](doc/en/quickstart.md)

## Document
1. [Build the Project](doc/en/build.md)

[More Topics](doc/en/contents.md)

## License
[MIT](LICENSE) License
