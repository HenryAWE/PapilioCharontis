其他语言：[English](README.md)
# 🦋 引蝶座（Papilio Charontis）
[![Build](https://github.com/HenryAWE/PapilioCharontis/actions/workflows/build.yml/badge.svg)](https://github.com/HenryAWE/PapilioCharontis/actions/workflows/build.yml)

功能灵活的 C++ 格式化库，主要为国际化（i18n）场景设计。同时，它也可以作为 `printf`、`<iostream>` 和 `std::format` 的替代品。

通过内嵌的脚本控制输出内容，将单复数、阴阳性等逻辑与程序逻辑解耦，完全由翻译来掌控。

例如，如果要以传统方式正确输出，代码可能如下所示：
```c++
if(n == 1)
     print(translate("Found {} file"), n);
else
     print(translate("Found {} files"), n);
```
该代码的输出逻辑与其他逻辑混合在一起，并且它无法处理复数形式数量与英语不同的语言（如俄语）。

有了这个库，代码可以改为：
```c++
print(translate("Found {0} file{${0}!=1:'s'}"), n);
```

该库不仅支持基于数值的输出，还支持基于复杂条件的输出。
```c++
struct person
{
    string name;
    int gender;

    bool is_female() const;
};

/* 一些用于注册属性的代码，详情请参阅文档 */

// 示例用法
person p = /* ... */;
papilio::format("{$ {0.is_female} ? 'She' : 'He'} is a nice person.", p);
// 如果 is_female() 返回 true，结果将会是 "She is a nice person."
// 否则结果将会是 "He is a nice person."
```

## 概览
### 核心特性：内嵌脚本
使用内嵌脚本控制输出，例如对某个数字使用单词的复数形式。

#### 示例 1：
根据警告的数量，决定是否使用“warning”一词的复数形式。
```c++
papilio::format("{0} warning{$ {0}>1 ? 's'}", 1); // 返回 "1 warning"
papilio::format("{0} warning{$ {0}>1 ? 's'}", 2); // 返回 "2 warnings"
```
#### 示例 2：
根据物品的数量决定单词的形式。
```c++
// 英文
std::string_view fmt_en =
    "There {$ {0}!=1 ? 'are' : 'is'} {0} apple{$ {0}!=1 ? 's'}";
papilio::format(fmt_en, 1); // 返回 "There is 1 apple"
papilio::format(fmt_en, 2); // 返回 "There are 2 apples"

// 法文
std::string_view fmt_fr =
    "Il y a {0} pomme{$ {0}>1 ? 's'}";
papilio::format(fmt_fr, 1); // 返回 "Il y a 1 pomme"
papilio::format(fmt_fr, 2); // 返回 "Il y a 2 pommes"

// 中文（无变形）
std::string_view fmt_zh =
    "有 {0} 个苹果";
papilio::format(fmt_zh, 1); // 返回 "有 1 个苹果"
papilio::format(fmt_zh, 2); // 返回 "有 2 个苹果"
```

#### 更多示例
`example/` 目录中有更多的示例。

### 兼容 {fmt} 与 C++ 20 `<format>` 的使用方法
本库和 {fmt} 与 `<format>` 在基础格式化语法上一致。
```c++
// 格式化说明符
papilio::format("#{:08x}", 0xff);                  // 返回 "#000000ff"
// 用于输出花括号的转义序列
papilio::format("{{plain text}}");                 // 返回 "{plain text}"
// 重定位参数
papilio::format("{1} and {0}", "second", "first"); // 返回 "first and second"
```
更多关于格式化说明符的信息请参阅 [内建格式化器（Formatter）](doc/zh-CN/builtin_formatter.md)。

{fmt} 的具名参数也受支持。
```c++
using namespace papilio::literals;
papilio::format("{text} and {0}", "world", "text"_a = "hello");
// 返回 "hello and world"
```
如果你不想使用 `using namespace`，可以用 `papilio::arg("text", "hello")` 代替。

#### 额外功能
无需额外代码即可支持枚举（enum）值转字符串。

```c++
enum animal
{
    cat = 1,
    dog
};

papilio::format("{}", cat);   // 返回 "cat"
papilio::format("{}", dog);   // 返回 "dog"
papilio::format("{:d}", cat); // 返回 "1"
```

### 访问成员
支持访问成员属性与基于整数，范围或字符串下标访问。
```c++
papilio::format("length of \"{0}\" is {0.length}", "hello");
// 返回 "length of "hello" is 5"
papilio::format("{[:5]:}", "hello world");  // 返回 "hello"
papilio::format("{[-5:]:}", "hello world"); // 返回 "world"
papilio::format("{[0]:}", "hello world");   // 返回 "h"
papilio::format("{[-1]:}", "hello world");  // 返回 "d"
```
详细信息请参阅 [内建访问器（Accessor）](doc/zh-CN/builtin_accessor.md)。

#### Unicode 支持
可以在格式化函数中方便地操作 Unicode 字符串。
```c++
papilio::format("{[:2]}", "你好，世界");
// 返回 "你好" 而不是无法表示有意义字符的前两个字节
papilio::format("长度：{0.length}；大小：{0.size}", "你好，世界");
// 返回 "长度：5；大小：15"
```
注意：要运行上文的代码，需要保证代码使用了 UTF-8 编码保存，并且设置了正确的编译器选项（如 MSVC 的 `/utf-8` 选项）。你可以使用 `u8` 前缀以强制字符串使用 UTF-8 编码。

### 对于 `wchar_t`、`char8_t`、`char16_t` 与 `char32_t` 的格式化支持
```c++
papilio::format(L"{}", true);  // 返回 L"true"
papilio::format(u8"{}", true); // 返回 u8"true"
papilio::format(u"{}", true);  // 返回 u"true"
papilio::format(U"{}", true);  // 返回 U"true"
```
注意：C++ 新标准添加的 `char8_t`、`char16_t`、`char32_t` 的支持需要包含单独的头文件 `<papilio/xchar.hpp>`。此外，这些字符类型不支持特定于本地环境（locale-specific）的格式化。

## 文档
1. [构建](doc/zh-CN/build.md)：如何构建项目
2. [常见问题（FAQ）](doc/zh-CN/faq.md)
3. [示例](doc/zh-CN/examples.md)
4. [脚本介绍](doc/zh-CN/script.md)
5. [内建格式化器（Formatter）](doc/zh-CN/builtin_formatter.md)：常见类型的内建格式化器，你可以在这里找到常见类型的格式化说明的文档
6. [内建访问器（Accessor）](doc/zh-CN/builtin_accessor.md)：常见类型的内建访问器
7. [自定义格式化器（Formatter）](doc/zh-CN/formatter.md)：为自定义类型添加格式化支持，使得它们能够被输出
8. [自定义访问器（Accessor）](doc/zh-CN/accessor.md)：为自定义类添加索引、切片与属性支持

## 支持的平台和已知限制
该库的核心模块能跨平台。操作系统相关的 API 支持 Windows 和 Linux，理论上可以在 Mac 上使用。

某些功能由非标准 C++ 实现，但保证在受支持的编译器上经过 CI 测试，能获得正确的结果。

### 支持的编译器
- MSVC 19.3+（Visual Studio 2022）
- GCC 12+
- Clang 15+（使用 libc++）
- Clang 16+（使用 libstdc++，参见注意事项）

注意：使用 libstdc++ 的 Clang 16 ~ 17 仅支持在 C++20 下编译，需要 Clang 18+ 才能在 C++23 下使用此库。

## 许可证
[MIT](LICENSE) 许可证
