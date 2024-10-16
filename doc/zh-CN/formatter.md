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

# ADL `format`
当自定义格式化器不存在时，将会使用 ADL 查找符合要求的 `format` 函数：
```c++
// 简易 ADL format
class use_adl
{
public:
    // ...

private:
    template <typename Context>
    friend auto format(const use_adl& val, Context& ctx)
    {
        using namespace papilio;

        using context_t = format_context_traits<Context>;
        context_t::append(ctx, "ADL");

        return ctx.out();
    }
};

// 带参数的 ADL format
class use_adl_ex
{
public:
    // ...

private:
    template <typename ParseContext, typename FormatContext>
    friend auto format(const use_adl_ex& val, ParseContext& parse_ctx, FormatContext& fmt_ctx)
    {
        using namespace papilio;

        bool use_uppercase = false;

        {
            auto it = parse_ctx.begin();
            if(*it == U'S')
            {
                use_uppercase = true;
                ++it;
            }

            parse_ctx.advance_to(it);
        }

        using context_t = format_context_traits<FormatContext>;
        if(use_uppercase)
            context_t::append(fmt_ctx, "ADL (EX)");
        else
            context_t::append(fmt_ctx, "adl (ex)");

        return fmt_ctx.out();
    }
};
```
用户代码：
```c++
format("{}", use_adl{}); // 返回 "ADL"

format("{}", use_adl_ex{}); // 返回 "adl (ex)"
format("{:S}", used_adl_ex{}); // 返回 "ADL (EX)"
```

# `operator<<` 重载
如果上述格式化方式某个类型均未实现，但该类型实现了用于流式输出的老式 `operator<<` 重载，则该重载将会被用于输出。

# 禁用格式化器（Disabled Formatter）
显式阻止某个类型被格式化：
```c++
class format_disabled
{
public:
    // ...
};

namespace papilio
{
    template <typename CharT>
    struct formatter<format_disabled, CharT> : public disabled_formatter
    {};
}
```
即使 `format_disabled` 类实现了如 `operator<<` 或 ADL `format` 的其他输出方法，其也将无法被格式化。
