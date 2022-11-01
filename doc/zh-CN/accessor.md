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
    struct accessor
    {
        using has_index = void;
        
        static format_arg get(const my_type& val, indexing_value::index_type i)
        {
            if(i < 0 || i >= 10)
                return format_arg();
            return format_arg(val.values[i]);
        }
    };
}
```

## 更多功能
### 属性
```c++
static format_arg get_attr(const my_type& val, const attribute_name& attr)
{
    using namespace std::literals;
    if(attr == "size"sv)
        return format_arg(10);
    else
        throw invalid_attribute(attr);
}
```
