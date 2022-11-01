# Built-in Formatter
## Format Specification for Common Types
Used by fundamental types, character and string
```
fill-and-align sign # 0 width .precision L type
```
These arguments are all optional

### Fill and Align
Fill can be any character, followed by align option which is one of the `<`, `>` and `^`  
Align Option:
- `<`: Force the output to align to the beginning of available space. This is the default option for outputting non-integer or non-floating point values.
- `>`: Force the output to align to the ending of available space. This is the default option for outputting integer and floating point values.
- `^`: Force the output to align to the middle of available space. Given `n` characters waiting for insertion, this option will insert `n / 2` characters before the output and `n / 2 + n % 2` characters after the output.

### Sign, # and 0
Sign options:
- `+`: Use sign for both negative and non-negative number. Output `+` sign before non-negative value.
- `-`: Only negative number use sign. This is the default behavior
- ` ` (space): Use a leading space for non-negative number and use `-` for negative number.

The sign options are available for infinite values and NaN:
```c++
double inf = std::numeric_limits<double>::infinity();
double nan = std::numeric_limits<double>::quiet_NaN();
papilio::format("{0:},{0:+},{0:-},{0: }", inf); // "inf,+inf,inf, inf"
papilio::format("{0:},{0:+},{0:-},{0: }", nan); // "nan,+nan,nan, nan"
```

`#` Optional will enable the alternate form
- Integral type: When use binary, octal or hexadecimal for displaying, this option will insert prefix (`0b`, `0o` or `0x`) in the output.
- Floating type: not implemented yet

*NOTE: Different from `<format>` of C++ 20, this library use `0o` as the prefix for octal.*

### Width and Precision
Width is a decimal positive integer and can be dynamically specified by a replacement field (`{}`)  
Precision is also a decimal positive integer and be dynamically specified by a replacement field (`{}`).  Precision is only available for floating point and string. For floating point, this value determines the precision for formatting output. For string, this value determines the upper bound of the estimated width (see below) of output string.  
When width or precision values are dynamically specified by replacement fields, an exception will be thrown for any illegal value.
```c++
float pi = 3.14f;
papilio::format("{:.10f}", pi); // "  3.140000"
papilio::format("{:.5f}", pi); // "3.14000"
papilio::format("{:10.5f}", pi); // "   3.14000"
```
For calculating width of a character, if its Unicode value is in the following intervals, its value will be 2, otherwise its value will be 1:  
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

### L (Locale-Specific Formatting)
Not implemented yet.

### Type
#### String
- None, `s`: Copy the string to the output.

#### Integral Type (Except `bool` type)
- `b`: Binary output.
- None, `d`: Decimal output.
- `o`: Octal output.
- `x`: Hexadecimal output.

### Character Type
- None, `c`: Copy the string to the output.
- `b`, `d`, `o`, `x`: Use integer representation types

### `bool` Type
- None, `s`: Copy textual representation (`true` or `false`) to the output.
- `b`, `d`, `o`, `x`: Use integer representation types with `static_cast<unsigned int>(value)`.

### Floating Point
- `a`：If precision is specified, produces output as if by calling `std::to_chars(first, last, value, std::chars_format::hex, precision)`, where `precision` is the specified value; otherwise, the output is produced as if by calling `std::to_chars(first, last, value, std::chars_format::hex)`.
- `e`：Produces output as if by calling `std::to_chars(first, last, value, std::chars_format::scientific, precision)`, where `precision` is the specified precision, or `6` if it is not specified.
- `f`：Produces output as if by calling `std::to_chars(first, last, value, std::chars_format::fixed, precision)`, where `precision` is the specified precision, or `6` if it is not specified.
- `g`：If precision is specified, produces output as if by calling `std::to_chars(first, last, value, std::chars_format::general, precision)`, where  `precision` is the specified value; otherwise, the output is produced as if by calling `std::to_chars(first, last, value)`.
- None: If precision is specified, produces output as if by calling `std::to_chars(first, last, value, std::chars_format::general, precision)`, where `precision` is the specified value; otherwise, the output is produced as if by calling `std::to_chars(first, last, value)`.

Infinite values and NaN are formatted to `inf` and `nan`, respectively.

### Pointer
- None, `p`: Use hexadecimal  integer representation with  `static_cast<std::uniptr_t>(value)`, adding prefix `0x` into the output.

## Chrono Types
*NOTE: The formatter support for types involved in this section needs to include the header file `<papilio/util/chrono.hpp>`.*

### `std::tm` (In Header File `<ctime>`)
- None: Produces output as if by calling [`std::asctime`](https://en.cppreference.com/w/cpp/chrono/c/asctime).
- Format String: Produces output as if by calling [`std::strftime`](https://en.cppreference.com/w/cpp/chrono/c/strftime) with the format string.

```c++
std::tm t{};
t.tm_year = 2022 - 1900;
t.tm_mday = 1;
t.tm_wday = 6;

papilio::format("{}", t); // "Sat Jan  1 00:00:00 2022\n"
papilio::format("{:%H:%M:%S}", t); // "00:00:00"
```