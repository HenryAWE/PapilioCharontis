其他语言：[English](README.md)
# 引蝶座（Papilio Charontis）
功能灵活的 C++ 格式化库，可以作为 `printf`，`iostream` 和 `std::format` 的替代品。

## 概览
### 核心特性：脚本
使用内嵌脚本控制输出，例如对某个数字使用单词的复数形式。

#### 示例 1：
根据警告的数量，决定是否使用“warning”一词的复数形式。
```c++
papilio::format("{0} warning{${0}>1:'s'}", 1); // 返回 "1 warning"
papilio::format("{0} warning{${0}>1:'s'}", 2); // 返回 "2 warnings"
```
#### 示例 2：
根据苹果的数量决定动词的形式。
```c++
std::string_view fmt =
    "There"
    " {${0} != 1: 'are' : 'is'} "
    "{0}"
    " apple{${0} != 1: 's'}";
papilio::format(fmt, 1); // 返回 "There is 1 apple"
papilio::format(fmt, 2); // 返回 "There are 2 apples"
```

### 访问成员
支持索引（整数或字符串）、切片和访问成员属性。
```c++
papilio::format("length of \"{0}\" is {0.length}", "hello");
// 返回 "length of "hello" is 5"
papilio::format("{[:5]:}", "hello world"); // 返回 "hello"
papilio::format("{[-5:]:}", "hello world"); // 返回 "world"
papilio::format("{[0]:}", "hello world"); // 返回 "h"
papilio::format("{[-1]:}", "hello world"); // 返回 "d"
```

### 与 C++ 20 `<format>` 类似的功能
```c++
papilio::format("{}", 10); // 返回 "10"
papilio::format("#{:08x}", 0xff); // 返回 "#000000ff"
papilio::format("{1} and {0}", "second", "first"); // 返回 "first and second"
using namespace papilio::literals;
papilio::format("{text} and {0}", "world", "text"_a = "hello"); // 返回 "hello and world"
// 如果你不想使用 using namespace，可以用 papilio::arg("text", "hello") 代替
```

### Unicode 支持
你可以在格式化函数中方便地使用 Unicode 字符串。
```c++
papilio::format("{[:2]}", "你好，世界");
// 返回 "你好"，而不是返回无法表达有意义字符的前两个字节
papilio::format("长度：{0.length}；大小：{0.size}", "你好，世界");
// 返回 "长度：5；大小：15"
```
注意：要运行上文的代码，需要保证代码使用了 UTF-8 编码保存，并且设置了正确的编译器选项（如 MSVC 的 `/utf-8` 选项）。你可以使用 `u8` 前缀以强制字符串使用 UTF-8 编码。

## 文档
1. [构建项目](doc/zh-CN/build.md)
2. [脚本介绍](doc/zh-CN/script.md)
3. [常见问题（FAQ）](doc/zh-CN/faq.md)

[更多主题](doc/zh-CN/contents.md)

## 许可证
[MIT](LICENSE) 许可证
