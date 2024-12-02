# 自定义访问器（Accessor）
## 基础版本
```c++
class my_type
{
public:
    int values[10];
};

namespace papilio
{
    template <>
    struct accessor<my_type>
    {
        static format_arg index(const my_type& val, ssize_t i)
        {
            if(i < 0 || i >= 10)
                return format_arg();

            return val.values[i];
        }
    };
}
```

## 更多功能
### 属性
```c++
namespace papilio
{
    template <>
    struct accessor<my_type>
    {
        static format_arg attribute(const my_type& val, const attribute_name& attr)
        {
            using namespace std::literals;

            if(attr == "size"sv)
                return 10;

            throw_invalid_attribute(attr);
        }
    };
}
```
