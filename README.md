Other language: [中文](README.zh-CN.md)
# Papilio Charontis
A flexible formatting library for C++. It can be used as a replacement for `printf`, `iostream` and `std::format`.

## Overview
### Main Feature: Scripting
You can use the embedded script to control the output, such as using plural form of a word for a certain number.

#### Example 1:
Based on the number of warnings, decide whether to use the plural form of the word "warning".
```c++
papilio::format("{0} warning{${0}>1:'s'}", 1); // Returns "1 warning"
papilio::format("{0} warning{${0}>1:'s'}", 2); // Returns "2 warnings"
```
#### Example 2:
Determine the form of verb based on the number of apples.
```c++
std::string_view fmt =
    "There"
    " {${0} != 1: 'are' : 'is'} "
    "{0}"
    " apple{${0} != 1: 's'}";
papilio::format(fmt, 1); // Returns "There is 1 apple"
papilio::format(fmt, 2); // Returns "There are 2 apples"
```

### Accessing Member
Support indexing (integer or string), slicing and accessing member attributes.
```c++
papilio::format("length of \"{0}\" is {0.length}", "hello");
// Returns "length of "hello" is 5"
papilio::format("{[:5]:}", "hello world"); // Returns "hello"
papilio::format("{[-5:]:}", "hello world"); // Returns "world"
papilio::format("{[0]:}", "hello world"); // Returns "h"
papilio::format("{[-1]:}", "hello world"); // Returns "d"
```

### Similar Functionalities to `<format>` in C++ 20
```c++
papilio::format("{}", 10); // Returns "10"
papilio::format("#{:08x}", 0xff); // Returns "#000000ff"
papilio::format("{1} and {0}", "second", "first"); // Returns "first and second"
using namespace papilio::literals;
papilio::format("{text} and {0}", "world", "text"_a = "hello"); // Returns "hello and world"
// You can use papilio::arg("text", "hello") instead if you don't want using namespace
```

### Unicode Support
You can conveniently use Unicode string in formatting function.
```c++
papilio::format("{[:2]}", "你好，世界");
// Returns "你好" instead of first two bytes which cannot represent a valid character
papilio::format("Length: {0.length}; Size: {0.size}", "你好，世界");
// Returns "Length: 5; Size: 15"
```
NOTE: In order to run the above code, you need to make sure your code is saved in UTF-8 encoding and the correct compiler flags is set (like `/utf-8` of MSVC). You can use the `u8` prefix to force the string to use UTF-8 encoding.

## Documentation
1. [Build the Project](doc/en/build.md)
2. [Introduction to the Script](doc/en/script.md)
3. [Frequently Asked Questions](doc/en/faq.md)

[More Topics](doc/en/contents.md)

## License
[MIT](LICENSE) License
