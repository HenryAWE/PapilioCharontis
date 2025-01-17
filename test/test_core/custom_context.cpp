#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio/formatter/tuple.hpp>
#include <papilio_test/setup.hpp>

namespace test_core
{
template <typename T>
class my_float_formatter : public papilio::float_formatter<T, char>
{
    using my_base = papilio::float_formatter<T, char>;

public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        papilio::std_formatter_parser<ParseContext, true> parser;
        auto&& [data, it] = parser.parse(ctx, U"f");

        my_base::set_data(data);

        return it;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        std::string str;

        if(std::isnan(val))
        {
            str = "NaN";
        }
        else if(std::isinf(val))
        {
            str.reserve(8);
            str += std::signbit(val) ? '-' : '+';

            // '∞'
            const papilio::utf::codepoint inf_ch(U'\x221e');

            inf_ch.append_to(str);
        }

        if(str.empty())
            return my_base::format(val, ctx);
        else
        {
            papilio::string_formatter<char> fmt;
            fmt.set_data(this->data());
            return fmt.format(str, ctx);
        }
    }
};

template <typename OutputIt>
class custom_context
{
public:
    using char_type = char;
    using iterator = OutputIt;
    using format_args_type = papilio::basic_format_args_ref<custom_context, char>;

    template <typename AnotherOutputIt>
    struct rebind
    {
        using type = custom_context<AnotherOutputIt>;
    };

    template <typename T>
    using formatter_type = std::conditional_t<
        std::floating_point<T>,
        my_float_formatter<T>,
        papilio::select_formatter_t<T, custom_context>>;

    custom_context(papilio::locale_ref loc, iterator it, const format_args_type& args)
        : m_loc(loc), m_it(std::move(it)), m_args(args) {}

    iterator out() const
    {
        return m_it;
    }

    void advance_to(iterator it)
    {
        m_it = std::move(it);
    }

    const format_args_type& get_args() const noexcept
    {
        return m_args;
    }

    std::locale getloc() const
    {
        return m_loc;
    }

    papilio::locale_ref getloc_ref() const
    {
        return m_loc;
    }

private:
    papilio::locale_ref m_loc;
    iterator m_it;
    format_args_type m_args;
};
} // namespace test_core

TEST(custom_context, custom_context)
{
    using test_core::custom_context;

    using custom_ctx_type = custom_context<std::back_insert_iterator<std::string>>;

    // Redirected type (float)
    {
        std::string buf;
        papilio::basic_dynamic_format_args<custom_ctx_type> args;

        custom_ctx_type ctx(papilio::locale_ref{}, std::back_inserter(buf), args);

        using context_t = papilio::format_context_traits<custom_ctx_type>;

        static_assert(papilio::formattable_with<float, custom_ctx_type>);

        context_t::format_to(
            ctx,
            "{}, {}, {}, {}",
            3.14f,
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN()
        );

        EXPECT_EQ(buf, "3.14, +∞, -∞, NaN");

        buf.clear();

        context_t::format_to(
            ctx,
            "{:*^6}, {:*^4}, {:*^4}, {:*^5}",
            3.14f,
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN()
        );

        EXPECT_EQ(buf, "*3.14*, *+∞*, *-∞*, *NaN*");
    }

    // Ordinary types
    {
        std::string buf;
        papilio::basic_dynamic_format_args<custom_ctx_type> args;

        custom_ctx_type ctx(papilio::locale_ref{}, std::back_inserter(buf), args);

        using context_t = papilio::format_context_traits<custom_ctx_type>;

        static_assert(papilio::formattable_with<int, custom_ctx_type>);
        static_assert(papilio::formattable_with<papilio::utf::string_container, custom_ctx_type>);

        context_t::format_to(
            ctx,
            "{}",
            1013
        );
        EXPECT_EQ(buf, "1013");

        buf.clear();

        context_t::format_to(
            ctx,
            "{:*^9}",
            "hello"
        );
        EXPECT_EQ(buf, "**hello**");
    }

    // Nested formatter
    {
        std::string buf;
        papilio::basic_dynamic_format_args<custom_ctx_type> args;

        custom_ctx_type ctx(papilio::locale_ref{}, std::back_inserter(buf), args);

        using context_t = papilio::format_context_traits<custom_ctx_type>;

        static_assert(papilio::formattable_with<std::pair<float, float>, custom_ctx_type>);

        using limits_t = std::numeric_limits<float>;
        context_t::format_to(
            ctx,
            "{}",
            std::pair<float, float>(limits_t::infinity(), limits_t::quiet_NaN())
        );

        EXPECT_EQ(buf, "(+∞, NaN)");
    }
}
