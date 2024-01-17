# Script
Script used to control the formatting output. It a superset of the replacement fields from the standard library.  
If you need to directly embed some complex script into C++ code, the [raw string literal](https://en.cppreference.com/w/cpp/language/string_literal) feature of C++ 11 is recommended.

## Replacement Field
Similar to the standard library and {fmt}, it is used to insert values into the output string.

## Scripted Field
Script field is similar to the replacement field. It is surrounded by a pair of brace (`{}`) and starts with a `$` sign. A script field must contains a condition statement and at least one branch. Condition statement and branches are separated by the colon (`:`).  
For example, `{$ {0} != 0: 'non-'}zero` and `{$ {0} != 0: 'non-zero' : 'zero'}` have valid script fields.

### Condition Statement
The condition statement is a Boolean expression. It can be comparison, logical not, or a value that convertible to `bool`. The value in the condition statement can be format arguments (`format_arg`) or constant values. The format arguments is referred to by the same syntax as the replacement field, but without the format specifiers.

1. Comparison
   | Operator | Meaning          |
   | -------: | :--------------- |
   |     `==` | Equal            |
   |     `!=` | Not equal        |
   |      `>` | Greater than     |
   |      `<` | Less than        |
   |     `>=` | Greater or equal |
   |     `<=` | Less or equal    |

2. Logical not
Logical not is preceding by an exclamation mark (`!`). It only has one operand.

3. Value that convertible to `bool`
   Non-zero numerical value and non-empty string are considered as `true`, otherwise, they are considered as `false`.
