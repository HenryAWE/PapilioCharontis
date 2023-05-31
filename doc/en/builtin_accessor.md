# Built-In Accessor
Built-in accessor for common types.

## String Type（`papilio::string_container`）
### Indexing by Integer
Forward index starts from `0` , returns null character when exceeding maximum index.  
Backward (Reverse) index starts from `-1` (similar to Python), also returns null character when exceeding maximum index.  
Example: Given string  `"hello world!"`, then

- `[0]`: Returns `'h'`.
- `[100]`: Returns null character.
- `[-1]`: Returns `'!'`.
- `[-100]`: Returns null character.

### Slicing
The left-closed, right-open interval `[begin, end)` consists of index pair `begin`, `end`  
Default values are `0` and `.length`, respectively
Example: Given string "hello world!"
- `[:]`: Returns `"hello world!"`
-  `[:-1]`: Returns `"hello world"`
-  `[6:-1]`: Returns `"world"`
-  `[-6:]`: Returns `"world!"`

### Attributes
- `size`：The number of *bytes* in the string.
- `length`：The number of *characters* in the string.
NOTE: For string containing non-ASCII characters, these two values will not be equal.  
For example, the `size` of string `"ü"` is `2`, but its `length` is `1`.

---
*NOTE: The accessor support for following types needs to include the header file `<papilio/util/stl_container.hpp>`.*

## Tuples (`tuple` and `pair`)
### Indexing by Integer
Forward index starts from `0` and returns null when exceeding maximum index  
Backward (Reverse) index starts from `-1` and returns null when exceeding maximum index  
Note: Outputting null value will cause an exception.

### Attributes
- `size`: Number of elements in tuple.

The type `pair` has two external attributes:  
- `first`: The first element of `pair`.
- `second`: The second element of `pair`.

## Contiguous Containers (`vector`, `array` and `span`)
### Indexing by Integer
Forward index starts from `0` and returns null when exceeding maximum index.  
Backward (reverse) index starts from ‘-1′ and returns null when exceeding maximum index.  
Note: Outputting null value will cause an exception.

### Attributes
- `size`: Size of the container.

## Associative Containers Whose Key Types Are Integer or String (`map<Integral, T>`, `map<String, T>`, etc.)
### Indexing by Integer or String
Will return the corresponding value in the container, or null value if the key doesn't exist.  
Note: Outputting null value will cause an exception.

### Attributes
- `size`: Size of the container.
