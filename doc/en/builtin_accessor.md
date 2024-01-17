# Built-In Accessor
Built-in accessor for common types.

## String Type（`papilio::utf::basic_string_container<CharT>`）
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
- `size`：The number of *elements* in the string. That is, the string is regarded as a container whose value type is `char` (or other character type), and the result is its number of elements.
- `length`：The number of *characters* in the string.  
For string containing non-ASCII characters, these two values may not be equal. For example, the `size` of string `"ü"` is `2`, but its `length` is `1`; for string `L"ü"` (`wchar_t` string), its `size` and `length` are both `1`.

---
*NOTE: The accessor support for following types needs to include the header file `<papilio/util/stl_container.hpp>`.*

## Tuples (`tuple` and `pair`)
### Indexing by Integer
Forward index starts from `0` and returns null when exceeding maximum index  
Backward (Reverse) index starts from `-1` and returns null when exceeding maximum index  
Note: Outputting null value will cause an exception.

### Attributes
- `size`: Number of elements in tuple (i.e. `std::tuple_size_v`).

The type `pair` has two additional attributes:  
- `first`: The first element of `pair`.
- `second`: The second element of `pair`.

## Contiguous Ranges (e.g. `std::vector`, `std::array` and `std::span`)
### Indexing by Integer
Forward index starts from `0` and returns null when exceeding maximum index.  
Backward (reverse) index starts from `-1` and returns null when exceeding maximum index.  
Note: Outputting null value will cause an exception.

### Attributes
- `size`: Size of the container.

## Associative Containers (`std::map`)
### Indexing by Integer or String
Will return the corresponding value in the container, or null value if the key doesn't exist. This function is only enabled when `key_type` of the container is integer or string.  

### Attributes
- `size`: Size of the container.

If the comparator of the container (i.e. the `key_compare` type) is specialization of `std::less<T>` or `std::greater<T>`, there are will be two additional attributes:
- `min`: The minimum element of the container, or null value if the container is empty.
- `max`: The maximum element of the container, or null value if the container is empty.
