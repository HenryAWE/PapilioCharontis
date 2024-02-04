# Script
Script is used to control the formatting output. It a superset of the replacement fields from the standard library. It consists of
- ordinary characters (characters except `{` and `}`), which are copied unchanged to the output,
- escape sequences `{{` and `}}`, which are replaced with `{` and `}` respectively in the output,
- replacement fields and scripted fields (see below).

## Replacement Field
Similar to the standard library and {fmt}, it is used to insert values into the output string. It has the following format:
1. `{arg-id}`: Replacement filed without a format specification
2. `{arg-id:format-spec}`: Replacement filed with a format specification

- `arg-id`: Specifies the index or the name of the argument; if it is omitted, the arguments are used in order. Using omitted arg-id after a specified one is an error.
  You can use `[]` operator and `.attribute` to access its member, e.g. `[0]`, `[0:5]`, `['name']` and `.size`. See [Built-In Accessor](./builtin_accessor.md) and [Custom Accessor](./accessor.md) for more information
- `format-spec`: The format specification defined by corresponding specialization of `papilio::formatter`. See [Built-In Formatter](./builtin_formatter.md) and [Custom Formatter](./formatter.md) for more information.

## Scripted Field
Script field is similar to the replacement field. It is surrounded by a pair of brace (`{}`) and starts with a `$` sign. A script field must contains a condition statement and at least one branch. Condition statement and branches are separated by the colon (`:`).  
For example, `{$ {0} != 0: 'non-'}zero` and `{$ {0} != 0: 'non-zero' : 'zero'}` have valid script fields.

### Conditional Statement
The conditional statement is a Boolean expression. It can be comparison, logical not, or a value that convertible to `bool`. The value in the condition statement can be format arguments (`format_arg`) or constant values.

1. Arguments and Constants
   - Arguments: Referring the argument by the replacement field without a format specification.
    - String: surrounded by single quotes (`'`), using escape sequences to include special characters:
      | Escape Sequence                   | Result              |
      | --------------------------------- | ------------------- |
      | `\'`                              | `'`(single quote)   |
      | `\\`                              | `\`                 |
      | `\c` (`c` is any other character) | `c` (no conversion) |

    - Floating points and integers: `3.14`, `10`, etc.

2. Comparison
   | Operator | Meaning          |
   | -------: | :--------------- |
   |     `==` | Equal            |
   |     `!=` | Not equal        |
   |      `>` | Greater than     |
   |      `<` | Less than        |
   |     `>=` | Greater or equal |
   |     `<=` | Less or equal    |

3. Logical not
Logical not is preceding by an exclamation mark (`!`). It only has one operand.

1. Value that convertible to `bool`
   Non-zero numerical value and non-empty string are considered as `true`, otherwise, they are considered as `false`.

### Branch
When the conditional statement evaluates to `true`, the first branch is executed, otherwise the second branch is executed. For script fields with only one branch, no output will be produced if the conditional statement evaluates to `false`.

The execution result of the branch is to output a string, which is defined by the same way as the conditional statement.
