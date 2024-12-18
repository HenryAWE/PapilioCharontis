# Built-In Formatter
The format specification of most built-in formatters are compatible with the usage of the corresponding parts of the standard library `<format>`.  
See the [standard library documentation](https://en.cppreference.com/w/cpp/utility/format/spec) for more detailed explanation.

## Format Specification for Common Types
Used by fundamental types, character and string.
```
fill-and-align sign # 0 width .precision L type
```
These arguments are all optional.

- **Note:**  
  In most of the cases the syntax is similar to the C-style `%`-formatting of `printf` family, with the addition of the `{}` and with `:` used instead of `%`.  
  For example, `%03.2f` can be translated to `{:03.2f}`

### Fill and Align
Fill can be any character, followed by align option which is one of the `<`, `>` and `^`.  
Align Option:
- `<`: Force the output to align to the beginning of available space. This is the default option for outputting non-integer or non-floating point values.
- `>`: Force the output to align to the ending of available space. This is the default option for outputting integer and floating point values.
- `^`: Force the output to align to the middle of available space. Given `n` characters waiting for insertion, this option will insert $\lfloor\frac{n}{2}\rfloor$ characters before the output and $\lceil\frac{n}{2}\rceil$ characters after the output.

### Sign, # and 0
Sign options:
- `+`: Use sign for both negative and non-negative number. Output `+` sign before non-negative value.
- `-`: Only negative number use sign. This is the default behavior.
- ` ` (space): Use a leading space for non-negative number and use `-` for negative number.

The sign options are available for infinite values and NaN:
```c++
double inf = std::numeric_limits<double>::infinity();
double nan = std::numeric_limits<double>::quiet_NaN();
papilio::format("{0:},{0:+},{0:-},{0: }", inf); // Returns "inf,+inf,inf, inf"
papilio::format("{0:},{0:+},{0:-},{0: }", nan); // Returns "nan,+nan,nan, nan"
```

The `#` option will enable the alternate form:
- Integral type: When use binary, octal or hexadecimal for displaying, this option will insert prefix (`0b`, `0` or `0x` for example) in the output.
- Floating type: not implemented yet

### Width and Precision
Width is a decimal positive integer and can be dynamically specified by a replacement field (`{}`).  
Precision is also a decimal positive integer and be dynamically specified by a replacement field (`{}`).  Precision is only available for floating point and string. For floating point, this value determines the precision for formatting output. For string, this value determines the upper bound of the estimated width (see below) of output string.  
When width or precision values are dynamically specified by replacement fields, an exception will be thrown for any illegal value.
```c++
float pi = 3.14f;
papilio::format("{:.10f}", pi); // "  3.140000"
papilio::format("{:.5f}", pi); // "3.14000"
papilio::format("{:10.5f}", pi); // "   3.14000"
```
For calculating width of a character, if its Unicode value is in the following intervals, its value will be 2, otherwise its value will be 1:  
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

### L (Locale-Specific Formatting)
This option is only available for some types. It may cause the output to be affected by locale.
- `bool` type: The output will use the result of `std::numpunct::truename()` or `std::numpunct::falsename()` to represent `true` or `false`.
- Integral type: Appropriate digit group separators will be inserted to the output.
- Floating point: Appropriate digit group separators and decimal point will be inserted to the output.

### Type
#### String
- None, `s`: Copy the string to the output.
- `?`: Copy escaped string (see below) to the output.

#### Integral Type (Except `bool` type)
- `b`: Binary output.
- `B`: Same as `b`, but prefix `0B` to the output with `#`.
- None, `d`: Decimal output.
- `o`: Octal output.
- `x`: Hexadecimal output.
- `X`: Same as `x`, but use uppercase letters and prefix `0X` to the output with `#`.

#### Character Type
- None, `c`: Copy the character to the output.
- `?`: Copy the escaped character (see below) to the output.
- `b`, `B`, `d`, `o`, `x`, `X`: Use integer representation types with `static_cast<std::uint32_t>(value)`.

#### `bool` Type
- None, `s`: Copy textual representation string (`true` or `false`) to the output.
- `b`, `B`, `d`, `o`, `x`, `X`: Use integer representation types with `static_cast<unsigned int>(value)`.

#### Floating Point
- `a`：If precision is specified, produces output as if by calling `std::to_chars(first, last, value, std::chars_format::hex, precision)`, where `precision` is the specified value; otherwise, the output is produced as if by calling `std::to_chars(first, last, value, std::chars_format::hex)`.
- `e`：Produces output as if by calling `std::to_chars(first, last, value, std::chars_format::scientific, precision)`, where `precision` is the specified precision, or `6` if it is not specified.
- `f`：Produces output as if by calling `std::to_chars(first, last, value, std::chars_format::fixed, precision)`, where `precision` is the specified precision, or `6` if it is not specified.
- `g`：If precision is specified, produces output as if by calling `std::to_chars(first, last, value, std::chars_format::general, precision)`, where  `precision` is the specified value; otherwise, the output is produced as if by calling `std::to_chars(first, last, value)`.
- None: If precision is specified, produces output as if by calling `std::to_chars(first, last, value, std::chars_format::general, precision)`, where `precision` is the specified value; otherwise, the output is produced as if by calling `std::to_chars(first, last, value)`.

Infinite values and NaN are formatted to `inf` and `nan`, respectively.

#### Pointer
- None, `p`: Use hexadecimal  integer representation with  `static_cast<std::uintptr_t>(value)`, adding prefix `0x` into the output.
- `P`: Same as `p`, but use uppercase letters.

#### Enumeration Type (`enum`)
- None, `s`: Copy the corresponding string of the enumeration value to the output.
- `b`, `B`, `d`, `o`, `x`, `X`: Use integer representation types with `static_cast<std::underlying_type_t<Enum>>(value)`.

Note: The `enum_name` function defined in `<papilio/utility.hpp>` uses compiler extension to retrieve string from enumeration value. It has following limitations:  
1. Requires compiler extension. If supported by the compiler, the implementation will define a `PAPILIO_HAS_ENUM_NAME` macro.
2. Only support enumeration values within the `[-128, 128)` range.
3. Output result of multiple enumerations with same value is compiler dependent.

# Formatting escaped characters and strings
A character or string can be formatted as escaped to make it more suitable for debugging or for logging.

For a character `C`:
- If `C` is one of the characters in the following table, the corresponding escape sequence is used.  
  | Character       | Escape sequence | Notes                                           |
  | --------------- | --------------- | ----------------------------------------------- |
  | Horizontal tab  | `\t`            |                                                 |
  | New line        | `\n`            |                                                 |
  | Carriage return | `\r`            |                                                 |
  | Double quote    | `\"`            | Used only if the output is double-quoted string |
  | Single quote    | `\'`            | Used only if the output is single-quoted string |
  | Backslash       | `\\`            |                                                 |

- If `C` and following characters form a sequence that is not printable.

- If `C` and following characters cannot form a valid code point. The hexadecimal digits will be used to represent the invalid sequence.

## Example
```c++
papilio::format("{:?} {:?}", '\'', '"'); // Returns "\\' \""
papilio::format("{:?}", "hello\n"); // Returns "hello\\n"
papilio::format("{:?}", std::string("\0 \n \t \x02 \x1b", 9)); // Returns "\\u{0} \\n \\t \\u{2} \\u{1b}"
// Invalid UTF-8
papilio::format("{:?}", "\xc3\x28"); // Returns "\\x{c3}("
```

# Date and Time
Formatter for date and time is provided by a separated header `<papilio/formatter/chrono.hpp>`

```
fill-and-align L chrono-spec
```

1. fill-and-align is same as the one for fundamental types.
2. `L` means using locale object. Otherwise, the output will be locale-independent or use C-locale.
3. chrono-spec: Format specification. It can be any characters other than `{` and `}`.

## `std::tm` from `<ctime>`
If the format specification is empty, the output result is similar to `asctime` but without the trailing newline. The the format specification is not empty, the formatter will forward the specification to `std::put_time` for converting it to string.

## Other types from `std::chrono`
The format specifier is started with a `%` and can be followed by the characters in the following list. Other characters will be directly copied to the output.

If the type to be formatted lacks of the required component, e.g. formatting a `std::chrono::hh_mm_ss` with specifier for formatting year, a corresponding exception will be thrown.

| Specifier | Meaning                                        |
| --------- | ---------------------------------------------- |
| `%%`      | Writes `%` to the output                       |
| `%n`      | Writes `\n` (newline) to the output            |
| `%t`      | Writes `\t` (tab) to the output                |
| `%c`      | Locale-dependent date and time representation. |

### Year
| Specifier           | Meaning                                                               |
| ------------------- | --------------------------------------------------------------------- |
| `%C`                | Writes year divided by 100, prefixed with 0 for single digit.         |
| `%Ec`               | Locale-dependent century format.                                      |
| `%y`                | Writes last two digits of the year, prefixed with 0 for single digit. |
| `%Y`                | Writes the year as number using `{:04d}` format.                      |
| `%Oy`, `%Ey`, `%EY` | Locale-dependent year representation.                                 |

### Month
| Specifier  | Meaning                                                                         |
| ---------- | ------------------------------------------------------------------------------- |
| `%b`, `%h` | Writes abbreviated name (`Jan`). Will use locale if possible.                   |
| `%B`       | Writes full name (`January`). Will use locale if possible.                      |
| `%m`       | Writes the month as number (January is `01`), prefixed with 0 for single digit. |
| `%Om`      | Locale-dependent month representation.                                          |

### Day
| Specifier    | Meaning                                                        |
| ------------ | -------------------------------------------------------------- |
| `%d`         | Writes the day of month, prefixed with 0 for single digit.     |
| `%e`         | Writes the day of month, prefixed with space for single digit. |
| `%Od`, `%Oe` | Locale-dependent day representation.                           |

### Day of the Week
| Specifier    | Meaning                                                       |
| ------------ | ------------------------------------------------------------- |
| `%a`         | Writes abbreviated name (`Mon`). Will use locale if possible. |
| `%A`         | Writes full name (`Monday`). Will use locale if possible.     |
| `%u`         | Writes the ISO weekday as number (1-7), where Monday is 1.    |
| `%w`         | Writes the weekday as number (0-6), where Sunday is 0.        |
| `%Ou`, `%Ow` | Locale-dependent weekday representation.                      |

### Week/Day of the Year
| Specifier    | Meaning                                           |
| ------------ | ------------------------------------------------- |
| `%j`         | Writes the day of the year using `{:03d}` format. |
| `%U`         | Not implemented yet.                              |
| `%W`         | Not implemented yet.                              |
| `%OU`, `%OW` | Not implemented yet.                              |

### Time of day
| Specifier         | Meaning                                                          |
| ----------------- | ---------------------------------------------------------------- |
| `%H`              | Writes the hour as number (24-hour clock) using `{:02d}` format. |
| `%I`              | Writes the hour as number (12-hour clock) using `{:02d}` format. |
| `%OH`, `%OI`      | Locale-dependent hour representation.                            |
| `%M`              | Writes the minute as number using `{:02d}` format.               |
| `%OM`             | Locale-dependent minute representation.                          |
| `%S`              | Writes the second as number using `{:02d}` format.               |
| `%OS`             | Locale-dependent second representation.                          |
| `%p`              | Writes `AM`/`PM`. Will use locale if possible.                   |
| `%R`              | Equivalent to `%H:%M`.                                           |
| `%T`              | Equivalent to `%H:%M:%S`.                                        |
| `%r`, `%X`, `%EX` | Locale-dependent time representation.                            |

### Duration Count
| Specifier | Meaning                                 |
| --------- | --------------------------------------- |
| `%Q`      | Writes the return value of `count()`    |
| `%q`      | Writes the unit suffix of the duration. |

Note: The unit suffix is same as [the standard library](https://en.cppreference.com/w/cpp/chrono/duration/operator_ltlt). It choose `us` instead of `μs` as suffix for the `std::micro` for portability.

### Time Zone
| Specifier    | Meaning                                                                        |
| ------------ | ------------------------------------------------------------------------------ |
| `%z`         | Writes the offset from UTC in the ISO 8601 format, e.g. `+0800`.               |
| `%Ez`, `%Oz` | Similar to `%z`, but insert a `:` between the hours and minutes, e.g. `+08:00` |
| `%Z`         | Writes the time zone abbreviation.                                             |
