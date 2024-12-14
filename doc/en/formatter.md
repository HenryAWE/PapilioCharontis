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

# Member `format`
If the custom formatter is not existed, the member `format` will be tried:
```c++
// Simple member format
class member_fmt
{
public:
    template <typename Context>
    auto format(Context& ctx) const
        -> typename Context::iterator
    {
        std::string_view str = "member";

        return std::copy(str.begin(), str.end(), ctx.out());
    }
};

// Member format with arguments
class member_fmt_ex
{
public:
    template <typename ParseContext, typename FormatContext>
    auto format(ParseContext& parse_ctx, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        bool upper = false;

        auto parse_it = parse_ctx.begin();
        if(parse_it != parse_ctx.end() && *parse_it != U'}')
        {
            if(*parse_it == U'U')
            {
                upper = true;
                ++parse_it;
            }
            else
                throw papilio::format_error("bad spec");

            parse_ctx.advance_to(parse_it);
        }

        std::string_view str = upper ? "MEMBER" : "member";

        return std::copy(str.begin(), str.end(), ctx.out());
    }
};
```
User code:
```c++
papilio::format("{}", member_fmt{}); // Returns "member"

papilio::format("{}", member_fmt_ex{}); // Returns "member"
papilio::format("{:U}", member_fmt_ex{}); // Returns "MEMBER"
```

# ADL `format`
If the custom formatter amd member `format` are not existed, the `format` found by ADL will be used:
```c++
// Simple ADL format
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

// ADL format with arguments
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
User code:
```c++
papilio::format("{}", use_adl{}); // Returns "ADL"

papilio::format("{}", use_adl_ex{}); // Returns "adl (ex)"
papilio::format("{:S}", used_adl_ex{}); // Returns "ADL (EX)"
```

# Overloaded `operator<<`
If a type does not implement the above format methods, but it has a overloaded `operator<<` for legacy stream output, that overload will be used for outputting.

# Disabled Formatter
Explicitly prevent a type from being formatted:
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
The class `format_disabled` will not be formatted even if it implements other output method like `operator<<` or ADL `format`.
