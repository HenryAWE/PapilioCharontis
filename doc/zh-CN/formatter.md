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
        template <typename ParseContext>
        auto parse(ParseContext& ctx)
        {
            auto it = ctx.begin();

            if(*it == U's')
            {
                m_as_str = true;
                ++it;
            }

            return it;
        }

        template <typename Context>
        auto format(const test_format::my_value& val, Context& ctx)
        {
            if(m_as_str)
            {
                using context_t = format_context_traits<Context>;
                context_t::append(ctx, val.ch, val.count);
                return ctx.out();
            }
            else
            {
                return format_to(ctx.out(), "({}, {})", val.ch, val.count);
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
