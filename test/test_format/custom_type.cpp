#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

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
    friend auto format(const use_adl&, Context& ctx)
    {
        using namespace papilio;

        using char_type = typename Context::char_type;

        using context_t = format_context_traits<Context>;
        context_t::append(ctx, PAPILIO_TSTRING_VIEW(char_type, "ADL"));

        return ctx.out();
    }
};

class use_adl_ex
{
private:
    // Hidden friend
    template <typename ParseContext, typename FormatContext>
    friend auto format(const use_adl_ex&, ParseContext& parse_ctx, FormatContext& fmt_ctx)
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
            context_t::append(fmt_ctx, PAPILIO_TSTRING_VIEW(char_type, "ADL (EX)"));
        else
            context_t::append(fmt_ctx, PAPILIO_TSTRING_VIEW(char_type, "adl (ex)"));

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

namespace test_format
{
class complex_spec
{
public:
};
} // namespace test_format

namespace papilio
{
template <>
class formatter<test_format::complex_spec>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        simple_formatter_parser<ParseContext, true> parser;
        auto [data, it] = parser.parse(ctx);
        m_data = data;

        return it;
    }

    template <typename Context>
    auto format(const test_format::complex_spec&, Context& ctx) const
    {
        using context_t = format_context_traits<Context>;

        const char* align_sign = "";
        switch(m_data.align)
        {
            using enum format_align;

        case left:
            align_sign = "<";
            break;
        case middle:
            align_sign = "^";
            break;
        case right:
            align_sign = ">";
            break;

        default:
        case default_align:
            // align_sign should be empty string
            break;
        }

        context_t::format_to(
            ctx,
            "{},{},{},{}",
            m_data.width,
            align_sign,
            m_data.fill_or(U' '),
            m_data.use_locale
        );

        return ctx.out();
    }

private:
    simple_formatter_data m_data;
};
} // namespace papilio

TEST(format, complex_spec)
{
    using test_format::complex_spec;
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{:<}", complex_spec{}), "0,<, ,false");
    EXPECT_EQ(PAPILIO_NS format("{:*<}", complex_spec{}), "0,<,*,false");
    EXPECT_EQ(PAPILIO_NS format("{:*<8}", complex_spec{}), "8,<,*,false");
    EXPECT_EQ(PAPILIO_NS format("{:*<8L}", complex_spec{}), "8,<,*,true");
}

namespace test_format
{
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
} // namespace test_format

TEST(format, member_format)
{
    using namespace papilio;
    using test_format::member_fmt;
    using test_format::member_fmt_ex;

    static_assert(formattable<member_fmt>);
    static_assert(formattable<member_fmt_ex>);

    EXPECT_EQ(PAPILIO_NS format("{}", member_fmt{}), "member");
    EXPECT_EQ(PAPILIO_NS format("{}", member_fmt_ex{}), "member");
    EXPECT_EQ(PAPILIO_NS format("{:U}", member_fmt_ex{}), "MEMBER");
    EXPECT_THROW((void)PAPILIO_NS format("{:I}", member_fmt_ex{}), format_error);
}

namespace test_format
{
struct person
{
    int gender; // 1 == female

    bool is_female() const
    {
        return gender == 1;
    }
};
} // namespace test_format

namespace papilio
{
template <typename Context>
struct accessor<test_format::person, Context>
{
    using char_type = typename Context::char_type;
    using format_arg_type = basic_format_arg<Context>;
    using attribute_name_type = basic_attribute_name<char_type>;

    static format_arg_type attribute(const test_format::person& p, const attribute_name_type& attr)
    {
        using namespace std::literals;

        if(attr == PAPILIO_TSTRING_VIEW(char_type, "is_female"))
        {
            return p.is_female();
        }

        throw_invalid_attribute(attr);
    }
};
} // namespace papilio

TEST(format, attributes)
{
    using test_format::person;
    using namespace papilio;

    {
        person p{.gender = 0};
        EXPECT_FALSE(p.is_female());

        EXPECT_EQ(
            format("{$ {0.is_female} ? 'She' : 'He'} is a nice person", p),
            "He is a nice person"
        );
        EXPECT_EQ(
            format(L"{$ {0.is_female} ? 'She' : 'He'} is a nice person", p),
            L"He is a nice person"
        );
    }

    {
        person p{.gender = 1};
        EXPECT_TRUE(p.is_female());

        EXPECT_EQ(
            format("{$ {0.is_female} ? 'She' : 'He'} is a nice person", p),
            "She is a nice person"
        );
        EXPECT_EQ(
            format(L"{$ {0.is_female} ? 'She' : 'He'} is a nice person", p),
            L"She is a nice person"
        );
    }
}
