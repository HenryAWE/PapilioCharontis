# 自定义格式化器（Formatter）
定义自定义类型：
```c++
class my_value
{
public:
    char ch;
    int count;
};
```
实现格式化器：  
- 对于无格式说明的默认情况，输出 `(ch, count)`
- 对于 `s` 格式说明，则输出 `count` 个 `ch`
```c++
namespace papilio
{
    template <>
    class formatter<test_format::my_value>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            std::string_view view(spec.begin(), spec.end());
            if(view == "s")
                m_as_str = true;
        }
        template <typename Context>
        void format(const test_format::my_value& val, Context& ctx)
        {
            format_context_traits traits(ctx);
            if(m_as_str)
                traits.append(val.ch, val.count);
            else
            {
                std::string result;
                result += '(';
                result += val.ch;
                result += ", ";
                result += std::to_string(val.count);
                result += ')';
                traits.append(result);
            }
        }

    private:
        bool m_as_str = false;
    };
}
```
用户代码：
```c++
my_value my_val('B', 3);
format("{}", my_val); // 返回 "(B, 3)"
format("{:s}", my_val); // 返回 "BBB"
```
