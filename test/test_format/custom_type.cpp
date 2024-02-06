#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>

namespace test_format
{
struct unformattable_type
{};

struct large_unformattable_type
{
    char dummy[1024]{};
};

struct custom_type
{
    int val = 0;

    custom_type(int v)
        : val(v) {}
};

struct large_custom_type
{
    int val = 0;

    large_custom_type(int v)
        : val(v) {}

    large_custom_type(const large_custom_type&) = default;

private:
    char dummy[1024]{};
};

struct complex_custom_type
{
    int val;

    complex_custom_type(int v)
        : val(v) {}

    complex_custom_type(const complex_custom_type&) = default;
};
} // namespace test_format

namespace papilio
{
template <>
class formatter<test_format::custom_type>
{
public:
    template <typename ParseContext, typename FormatContext>
    auto format(const test_format::custom_type& v, ParseContext&, FormatContext& ctx) const
    {
        return PAPILIO_NS format_to(ctx.out(), "custom_type.val={}", v.val);
    }
};

template <>
class formatter<test_format::large_custom_type>
{
public:
    template <typename ParseContext, typename FormatContext>
    auto format(const test_format::large_custom_type& v, ParseContext&, FormatContext& ctx) const
    {
        return PAPILIO_NS format_to(ctx.out(), "large_custom_type.val={}", v.val);
    }
};

template <>
class formatter<test_format::complex_custom_type>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        auto it = std::find(ctx.begin(), ctx.end(), U'}');

        m_spec = utf::string_ref(ctx.begin(), it);

        return it;
    }

    template <typename Context>
    auto format(const test_format::complex_custom_type& val, Context& ctx) const
        -> typename Context::iterator
    {
        if(m_spec.empty())
        {
            return PAPILIO_NS format_to(ctx.out(), "val={}", val.val);
        }
        else
        {
            using context_t = format_context_traits<Context>;
            for(int i = 0; i < val.val; ++i)
                context_t::append(ctx, m_spec);
            return ctx.out();
        }
    }

private:
    utf::string_ref m_spec;
};
} // namespace papilio

TEST(format, custom_type)
{
    using namespace papilio;
    using namespace test_format;

    static_assert(!formattable<unformattable_type>);
    static_assert(!formattable<large_unformattable_type>);
    static_assert(formattable<custom_type>);
    static_assert(formattable<large_custom_type>);
    static_assert(!formatter_traits<formatter<custom_type>>::parsable());
    static_assert(!formatter_traits<formatter<large_custom_type>>::parsable());

    {
        format_arg fmt_arg(unformattable_type{});
        EXPECT_FALSE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg(large_unformattable_type{});
        EXPECT_FALSE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg(custom_type(182376));
        EXPECT_TRUE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg(large_custom_type(182376));
        EXPECT_TRUE(fmt_arg.is_formattable());
    }

    EXPECT_EQ(format("{}", custom_type(182376)), "custom_type.val=182376");
    EXPECT_EQ(format("{}", large_custom_type(182376)), "large_custom_type.val=182376");

    {
        complex_custom_type val(2);
        static_assert(formattable<complex_custom_type>);
        static_assert(formatter_traits<formatter<complex_custom_type>>::parsable());

        EXPECT_EQ(PAPILIO_NS format("{}", val), "val=2");
        EXPECT_EQ(PAPILIO_NS format("{:-=-}", val), "-=--=-");
    }
}

namespace test_format
{
class use_adl
{
private:
    // Hidden friend
    template <typename Context>
    friend auto format(const use_adl& val, Context& ctx)
    {
        using namespace papilio;

        using char_type = typename Context::char_type;

        using context_t = format_context_traits<Context>;
        context_t::append(ctx, PAPILIO_TSTRING(char_type, "ADL"));

        return ctx.out();
    }
};

class use_adl_ex
{
private:
    // Hidden friend
    template <typename ParseContext, typename FormatContext>
    friend auto format(const use_adl_ex& val, ParseContext& parse_ctx, FormatContext& fmt_ctx)
    {
        using namespace papilio;

        using char_type = typename FormatContext::char_type;

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
            context_t::append(fmt_ctx, PAPILIO_TSTRING(char_type, "ADL (EX)"));
        else
            context_t::append(fmt_ctx, PAPILIO_TSTRING(char_type, "adl (ex)"));

        return fmt_ctx.out();
    }
};
} // namespace test_format

TEST(format, adl_format)
{
    using namespace papilio;

    static_assert(has_adl_format_v<test_format::use_adl>);
    static_assert(has_adl_format_v<test_format::use_adl_ex>);

    EXPECT_EQ(PAPILIO_NS format("{}", test_format::use_adl{}), "ADL");
    EXPECT_EQ(PAPILIO_NS format(L"{}", test_format::use_adl{}), L"ADL");

    EXPECT_EQ(PAPILIO_NS format("{}", test_format::use_adl_ex{}), "adl (ex)");
    EXPECT_EQ(PAPILIO_NS format(L"{}", test_format::use_adl_ex{}), L"adl (ex)");
    EXPECT_EQ(PAPILIO_NS format("{:S}", test_format::use_adl_ex{}), "ADL (EX)");
    EXPECT_EQ(PAPILIO_NS format(L"{:S}", test_format::use_adl_ex{}), L"ADL (EX)");
}
