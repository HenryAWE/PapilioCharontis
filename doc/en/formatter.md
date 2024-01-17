# Custom Formatter
Define a custom type:
```c++
class my_value
{
public:
    char ch;
    int count;
};
```
Implement the formatter:  
- Ouput `(ch, count)` for empty specification, which is the default behavior.
- Output `ch` for `count` times for `s` specification.
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
User code:
```c++
my_value my_val('B', 3);

format("{}", my_val); // Returns "(B, 3)"
format("{:s}", my_val); // Returns "BBB"
```