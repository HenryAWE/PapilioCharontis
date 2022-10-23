# Built-in Accessor
Built-in accessor for common types

## String Type（`papilio::string_container`）
### Indexing by Integer
Forward index starts from `0` , returns null character when exceeding maximum index  
Backward (Reverse) index starts from `-1` (similar to Python), also returns null character when exceeding maximum index  
Example: Given string  `"hello world!"`

- `[0]`: Returns  `'h'`
- `[100]`: Returns null character
- `[-1]`: Returns `'!'`
- `[-100]`: Returns null character

### Slicing
The left-closed, right-open interval `[begin, end)` consists of index pair `begin`, `end`  
Default values are `0` and `.length`, respectively
Example: Given string "hello world!"
- `[:]`: Returns `"hello world!"`
-  `[:-1]`: Returns `"hello world"`
-  `[6:-1]`: Returns `"world"`
-  `[-6:]`: Returns `"world!"`

### Attributes
- `size`：The number of *bytes* in the string
- `length`：The number of *characters* in the string
NOTE: For string containing non-ASCII characters, these two values will not be equal  
For example, the `size` of string `"ü"` is `2`, but its `length` is `1`
