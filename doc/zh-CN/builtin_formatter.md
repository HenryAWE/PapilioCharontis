# 内建格式化器（Formatter）
绝大部分的内建格式化器的格式说明都兼容标准库 `<format>` 里对应部分的用法。  
可以参考[标准库文档](https://zh.cppreference.com/w/cpp/utility/format/spec)获取更详细的说明。

## 常见类型的格式说明
用于基本类型、字符和字符串。
```
填充与对齐 符号 # 0 宽度 .精度 L 类型
```
这些参数都是可选的。

- **注**：  
  大多数情况下，这个语法与 C 式（`printf` 族函数）的 `%` 格式化类似。仅增加了 `{}`， 并用 `:` 替换掉 `%`。  
  例如 `%03.2f` 可被转换为 `{:03.2f}`。

### 填充与对齐
填充可以为任意字符，后随对齐选项为 `<`、`>` 与 `^` 之一。  
对齐选项：  
- `<`：强制输出对齐到可用空间的开始，在输出非整型或非浮点型时为默认选项。
- `>`：强制输出对齐到可用空间的末尾，在输出整型或浮点型时为默认选项。
- `^`：强制输出对齐到可用空间的中间；对于 `n` 个字符的输出而言，该选项将在输出的前面插入 $\lfloor\frac{n}{2}\rfloor$ 个字符，在后面插入 $\lceil\frac{n}{2}\rceil$ 个字符。

### 符号、# 与 0
符号选项：
- `+`：负数与非负数都要使用符号。在非负数前输出 `+` 号
- `-`：仅负数使用符号（默认行为）
- ` `（空格）：非负数使用前导空格，负数使用负号

符号选项对浮点型的无穷大与非数（NaN）有效：
```c++
double inf = std::numeric_limits<double>::infinity();
double nan = std::numeric_limits<double>::quiet_NaN();
papilio::format("{0:},{0:+},{0:-},{0: }", inf); // 返回 "inf,+inf,inf, inf"
papilio::format("{0:},{0:+},{0:-},{0: }", nan); // 返回 "nan,+nan,nan, nan"
```

`#` 选项将启用替换形式进行输出
- 整数类型：使用二进制、八进制或十六进制显示类型时，将向输出中插入前缀（如 `0b`、`0` 或 `0x`）
- 浮点类型：目前暂无实际效果

### 宽度与精度
宽度为十进制正整数，可以使用替换域（`{}`）动态指定  
精度同样为十进制正整数，可以使用替换域（`{}`）动态指定。精度只对浮点型和字符串类型有效。对于浮点类型，这个数值决定了格式化的输出精度；对于字符串类型，这个数值决定了输出字符串的估计宽度（见后文）的上界  
若使用替换域动态指定宽度或精度的数值，则会在遇到非法值时抛出异常
```c++
float pi = 3.14f;
papilio::format("{:.10f}", pi);  // "  3.140000"
papilio::format("{:.5f}", pi);   // "3.14000"
papilio::format("{:10.5f}", pi); // "   3.14000"
```
对于字符宽度计算，若其 Unicode 码点在下列范围内则宽度为 2，否则为 1：  
- `U+1100` - `U+115F`
- `U+2329` - `U+232A`
- `U+2E80` - `U+303E`
- `U+3040` - `U+A4CF`
- `U+AC00` - `U+D7A3`
- `U+F900` - `U+FAFF`
- `U+FE10` - `U+FE19`
- `U+FE30` - `U+FE6F`
- `U+FF00` - `U+FF60`
- `U+FFE0` - `U+FFE6`
- `U+1F300` - `U+1F64F`
- `U+1F900` - `U+1F9FF`
- `U+20000` - `U+2FFFD`
- `U+30000` - `U+3FFFD`

```c++
papilio::format("{:.^5s}", "文");       // ".文.."
papilio::format("{:.5s}", "文文文");    // "文文"
papilio::format("{:.<5.5s}", "文文文"); // "文文."
```

### L （依赖本地环境的格式化）
此选项仅对部分类型生效，将导致输出受到本地环境（locale）的影响。
- `bool` 类型：将使用 `std::numpunct::truename()` 或 `std::numpunct::falsename()` 获得的字符串来表示 `true` 或 `false`
- 整数类型：目前暂无实际效果
- 浮点类型：目前暂无实际效果

### 类型
#### 字符串类型
- 无、`s`：复制字符串到输出
- `?`: 复制转义过的字符串（见下文）到输出

#### 整数类型（除 `bool` 类型）
- `b`：二进制输出
- `B`：同 `b`，但是配合 `#` 使用时输出 `0B` 前缀
- 无、`d`：十进制输出
- `o`：八进制输出
- `x`：十六进制输出
- `X`：同 `x`，但是使用大写字母；配合 `#` 使用时输出 `0X` 前缀

#### 字符类型
- 无、`c`：复制字符到输出
- `?`: 复制转义过的字符（见下文）到输出
- `b`、`B`、`d`、`o`、`x`、`X`：以值 `static_cast<std::uint32_t>(value)` 使用整数表示字符

#### `bool` 类型
- 无、`s`：复制文本表示字符串（`true` 或 `false`）到输出
- `b`、`B`、`d`、`o`、`x`、`X`：以值 `static_cast<unsigned int>(value)` 使用整数表示

#### 浮点型
- `a`：若指定精度，则等价于 `std::to_chars(first, last, value, std::chars_format::hex, precision)` 产生的输出，其中 `precision` 为指定的精度；否则等价于 `std::to_chars(first, last, value, std::chars_format::hex)` 产生输出
- `e`：等价于 `std::to_chars(first, last, value, std::chars_format::scientific, precision)`  产生的输出，其中 `precision` 为指定的精度，若未指定精度则默认为 `6`
- `f`：等价于 `std::to_chars(first, last, value, std::chars_format::fixed, precision)` 产生的输出，其中 `precision` 为指定的精度，若未指定精度则默认为 `6`
- `g`：若指定精度，则等价于 `std::to_chars(first, last, value, std::chars_format::general, precision)` 产生的输出，其中 `precision` 为指定的精度；否则等价于 `std::to_chars(first, last, value)` 产生的输出
- 无：若指定精度，则等价于 `std::to_chars(first, last, value, std::chars_format::general, precision)` 产生的输出，其中 `precision` 为指定的精度；否则等价于 `std::to_chars(first, last, value)` 产生的输出。

无穷大和非数（NaN）分别格式化为 `inf` 与 `nan`

#### 指针类型
- 无、`p`：以值 `static_cast<std::uintptr_t>(value)` 使用十六进制整数表示，并向输出中添加前缀 `0x`
- `P`：同 `p`，但是使用大写字母

#### 枚举类型（`enum`）
- 无、`s`：复制枚举值对应的字符串到输出中
- `b`、`B`、`d`、`o`、`x`、`X`：以值 `static_cast<std::underlying_type_t<Enum>>(value)` 使用整数表示

注意：`<papilio/utility.hpp>` 中定义的 `enum_name` 函数使用编译器扩展从枚举值中获取字符串，它有以下限制：  
1. 需要编译器拓展。如果编译器支持，实现会定义 `PAPILIO_HAS_ENUM_NAME` 宏
2. 仅支持 `[-128, 128)` 范围内的枚举值
3. 具有相同值的多个枚举的输出结果取决于编译器

# 格式化输出转义过的字符与字符串
字符或字符串可以在格式化时进行转义，使其更适合用于调试或记录日志。

对于字符 `C` 而言：
- 如果 C 是下表中的字符之一，那么使用对应的转义序列：  
  | 字符     | 转义序列 | 注解                                    |
  | -------- | -------- | -------------------------------------- |
  | 横向制表 | `\t`     |                                        |
  | 换行     | `\n`     |                                        |
  | 回车     | `\r`     |                                        |
  | 双引号   | `\"`     | 仅会在输出是用双引号包围的字符串时使用     |
  | 单引号   | `\'`     | 仅会在输出是用单引号包围的字符串时使用     |
  | 反斜杠   | `\\`     |                                        |

- 如果 `C` 及其后续字符形成不可打印的序列。

- 如果 `C` 及其后续字符不能形成有效的码点。将使用十六进制数字来表示无效的序列。

## 示例
```c++
papilio::format("{:?} {:?}", '\'', '"'); // 返回 "\\' \""
papilio::format("{:?}", "hello\n"); // 返回 "hello\\n"
papilio::format("{:?}", std::string("\0 \n \t \x02 \x1b", 9)); // 返回 "\\u{0} \\n \\t \\u{2} \\u{1b}"
// 无效的 UTF-8
papilio::format("{:?}", "\xc3\x28"); // 返回 "\\x{c3}("
```
