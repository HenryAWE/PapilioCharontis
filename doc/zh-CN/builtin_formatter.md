# 内建格式化器（Formatter）
## 常见类型的格式说明
用于基本类型、字符和字符串。
```
填充与对齐 符号 # 0 宽度 .精度 L 类型
```
这些参数都是可选的。

### 填充与对齐
填充可以为任意字符，后随对齐选项为 `<`、`>` 与 `^` 之一。  
对齐选项：  
- `<`：强制输出对齐到可用空间的开始，在输出非整型或非浮点型时为默认选项
-  `>`：强制输出对齐到可用空间的末尾，在输出整型或浮点型时为默认选项。
-  `^`：强制输出对齐到可用空间的中间；对于 `n` 个待插入的字符而言，该选项将导致在输出的前面插入 $\lfloor\frac{n}{2}\rfloor$（即 C++ 中的 `n / 2`）个字符，在后面插入$\lceil\frac{n}{2}\rceil$（即 C++ 中的 `n / 2 + n % 2`）个字符

### 符号、# 与 0
符号选项：
- `+`：负数与非负数都要使用符号。在非负数前输出 `+` 号
- `-`：仅负数使用符号（默认行为）
- ` `（空格）：非负数使用前导空格，负数使用负号

符号选项对浮点型的无穷大与非数（NaN）有效：
```c++
double inf = std::numeric_limits<double>::infinity();
double nan = std::numeric_limits<double>::quiet_NaN();
papilio::format("{0:},{0:+},{0:-},{0: }", inf); // "inf,+inf,inf, inf"
papilio::format("{0:},{0:+},{0:-},{0: }", nan); // "nan,+nan,nan, nan"
```

`#` 选项将导致输出启用替换形式
- 整数类型：使用二进制、八进制或十六进制显示类型时，将插入前缀（`0b`、`0o` 或 `0x`）到输出中
- 浮点类型：目前暂无实际效果

**注意：和 C++ 20 的 `<format>` 不同，本库使用 `0o` 作为八进制的前缀**

### 宽度与精度
宽度为十进制正整数，可以使用替换域（`{}`）动态指定  
精度同样为十进制正整数，可以使用替换域（`{}`）动态指定。精度只对浮点型和字符串类型有效。对于浮点类型，这个数值决定了格式化的输出精度；对于字符串类型，这个数值决定了输出字符串的估计宽度（见后文）的上界  
若使用替换域动态指定宽度或精度的数值，则会在遇到非法值时抛出异常
```c++
float pi = 3.14f;
papilio::format("{:.10f}", pi); // "  3.140000"
papilio::format("{:.5f}", pi); // "3.14000"
papilio::format("{:10.5f}", pi); // "   3.14000"
```
对于字符宽度计算，若其 Unicode 码点位于下列范围内，则宽度为 2，否则为 1：  
- U+1100 - U+115F
- U+2329 - U+232A
- U+2E80 - U+303E
- U+3040 - U+A4CF
- U+AC00 - U+D7A3
- U+F900 - U+FAFF
- U+FE10 - U+FE19
- U+FE30 - U+FE6F
- U+FF00 - U+FF60
- U+FFE0 - U+FFE6
- U+1F300 - U+1F64F
- U+1F900 - U+1F9FF
- U+20000 - U+2FFFD
- U+30000 - U+3FFFD

```c++
papilio::format("{:.^5s}", "文"); // ".文.."
papilio::format("{:.5s}", "文文文"); // "文文"
papilio::format("{:.<5.5s}", "文文文"); // "文文."
```

### L （依赖本地环境的格式化）
目前暂无实际效果

### 类型
#### 字符串类型
- 无、`s`：复制字符串到输出

#### 整数类型（除 `bool` 类型）
- `b`：二进制输出
- 无、`d`：十进制输出
- `o`：八进制输出
- `x`：十六进制输出

### 字符类型
- 无、`c`：复制字符到输出
- `b`、`d`、`o`、`x`：使用整数表示字符

### `bool` 类型
- 无、`s`：复制文本表示（`true` 或 `false`）到输出
- `b`、`d`、`o`、`x`：以值 `static_cast<unsigned int>(value)` 使用整数表示字符

### 浮点型
- `a`：若指定精度，则等价于 `std::to_chars(first, last, value, std::chars_format::hex, precision)` 产生的输出，其中 `precision` 为指定的精度；否则等价于 `std::to_chars(first, last, value, std::chars_format::hex)` 产生输出
- `e`：等价于 `std::to_chars(first, last, value, std::chars_format::scientific, precision)`  产生的输出，其中 `precision` 为指定的精度，若未指定精度则默认为 `6`
- `f`：等价于 `std::to_chars(first, last, value, std::chars_format::fixed, precision)` 产生的输出，其中 `precision` 为指定的精度，若未指定精度则默认为 `6`
- `g`：若指定精度，则等价于 `std::to_chars(first, last, value, std::chars_format::general, precision)` 产生的输出，其中 `precision` 为指定的精度；否则等价于 `std::to_chars(first, last, value)` 产生的输出
- 无：若指定精度，则等价于 `std::to_chars(first, last, value, std::chars_format::general, precision)` 产生的输出，其中 `precision` 为指定的精度；否则等价于 `std::to_chars(first, last, value)` 产生的输出。

无穷大和非数（NaN）分别格式化为 `inf` 与 `nan`

### 指针类型
- 无、`p`：以值 `static_cast<std::uniptr_t>(value)` 使用十六进制整数表示，并添加前缀 `0x` 到输出

## 时间类型
**注意：本节中涉及的类型需要包含头文件 `<papilio/util/chrono.hpp>` 以获得格式化器支持**

### `std::tm` （位于头文件 `<ctime>` 中）
- 无：等价于使用 [`std::asctime`](https://zh.cppreference.com/w/cpp/chrono/c/asctime) 进行输出
- 格式字符串：等价于使用格式化字符串调用 [`std::strftime`](https://zh.cppreference.com/w/cpp/chrono/c/strftime) 进行格式化

```c++
std::tm t{};
t.tm_year = 2022 - 1900;
t.tm_mday = 1;
t.tm_wday = 6;

papilio::format("{}", t); // "Sat Jan  1 00:00:00 2022\n"
papilio::format("{:%H:%M:%S}", t); // "00:00:00"
```