# Script
Script used to control the formatting output.  
If you need to directly embed the script into C++ code, the [raw string literal](https://en.cppreference.com/w/cpp/language/string_literal) feature of C++ 11 is recommended.

## Arguments
- Integral Argument: Only contains decimal number, cannot starts by `0`. Index starts from `0`, corresponding to the argument in argument list.
- Named Argument: Contains visible characters, underscores (`_`) and numbers. The numbers cannot be the first character.

An exception will be thrown if the argument doesn't exist.

------
Expressed in EBNF:
```EBNF
nonzero digits = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
digits = "0" | nonzero digits ;
integral argument = nonzero digits, { digits } ;
identifier characters = ? all visible characters ? | "_" | digits ;
named argument = ( ? all visible characters ? | "_" ), { identifier characters } ;
argument = "$", ( integral argument | named argument ) ;
```

### Examples

1. `$0`：Refers to the first value in the argument list, whose index is `0`.
2. `$0 != $1`：Compares the first and second arguments for inequality.
3. `$name`：Refers to argument whose name is `name` in the argument list.

Remark: When using argument in replacement field, the leading “`$`” can be omitted  
For example, `{$0}` can be shortened as `{0}`, and`{$name}` can be shortened as `{name}`.

## Constant
Constant can be integer, floating point or string, such as `10` and `1.0`  
The content of string is surrounded by single quotes (`'`),  and can use the escape character backslash（`\`）to escape the origin meaning

| Escape Sequence | Result |
| :-------------: | :----: |
|      `\'`       |  `'`   |
|      `\\`       |  `\`   |

Examples:  
1. `'hello world'`
2. `'one\'s'` (Actual content `one's`)
3. `C:\\Windows` (Actual content `C:\Windows`)

## Compare and Logical Operators

The script supports six compare operations: equality (`==`), inequality (`!=`), greater than (`>`), greater equal (`>=`), less than (`<`), less equal (`<=`).  

The script only supports one logical operator, the logical not (`!`). Complex logical operators such as logical and or logical or are not supported.

## Control Flow
```
if cond-1 : 'result-1' elif cond-2: 'result-2' else: 'other'
```
If `cond-1` is true, it will return `'result-1'`; or, if `cond-2` is true, it will return `'result-2'`; otherwise, it will return `'other'`. There can be any number (can be zero) of `elif` branches in a control flow. The `else` branch is optional. When this branch is not specified, it will be implicitly defined as `else: ''`, returning an empty string (`''`).

Conditional expressions can be a comparison of two inputs or constants (`$0 == 'hello'`), a logical not of an input (`!$0`), or a direct converted to Boolean value from an input (`$0`)
| Input Type | Result |
| :------: |:--: |
| Integral or floating point values | `input != 0` |
| String | Whether the string is empty |
| Boolean values | No conversion |
