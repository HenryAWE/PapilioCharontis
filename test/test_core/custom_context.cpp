#include <gtest/gtest.h>
#include <papilio/core.hpp>
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

class custom_context : public papilio::format_context
{
    using my_base = papilio::format_context;

public:
    using iterator = my_base::iterator;
    using format_args_type = papilio::basic_format_args_ref<custom_context, char>;

    template <typename T>
    using formatter_type = std::conditional_t<
        std::floating_point<T>,
        my_float_formatter<T>,
        typename my_base::formatter_type<T>>;

    using my_base::my_base;

    custom_context(papilio::locale_ref loc, iterator it, const format_args_type& args)
        : my_base(loc, std::move(it), reinterpret_cast<const my_base::format_args_type&>(args)) {}
};
} // namespace test_core

TEST(custom_context, custom_context)
{
    using test_core::custom_context;

    {
        std::string buf;
        papilio::basic_dynamic_format_args<custom_context> args;

        custom_context ctx(papilio::locale_ref{}, std::back_inserter(buf), args);

        using context_t = papilio::format_context_traits<test_core::custom_context>;

        static_assert(papilio::formattable_with<float, custom_context>);

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
}
