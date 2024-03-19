#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio/format.hpp>

template <typename CharT>
class format_context_suite : public ::testing::Test
{
public:
    using string_type = std::basic_string<CharT>;
    using context_type = papilio::basic_format_context<
        std::back_insert_iterator<string_type>,
        CharT>;
    using args_type = papilio::basic_dynamic_format_args<context_type>;

    static context_type create_context(string_type& output)
    {
        return context_type{
            std::back_inserter(output),
            m_empty_args
        };
    }

private:
    // Shared placeholder
    static const inline args_type m_empty_args{};
};

using char_types = ::testing::Types<char, wchar_t>;
TYPED_TEST_SUITE(format_context_suite, char_types);

TYPED_TEST(format_context_suite, append)
{
    using namespace papilio;

    using context_type = typename TestFixture::context_type;

    typename TestFixture::string_type result{};
    context_type ctx = TestFixture::create_context(result);

    using context_t = format_context_traits<context_type>;

    {
        const auto str = PAPILIO_TSTRING(TypeParam, "1234");

        context_t::append(ctx, str);
        EXPECT_EQ(result, str);
    }

    result.clear();
    {
        context_t::append(ctx, '1', 4);

        const auto expected_str = PAPILIO_TSTRING(TypeParam, "1111");
        EXPECT_EQ(result, expected_str);
    }

    result.clear();
    {
        context_t::append(ctx, U'\u00c4', 2);

        const auto expected_str = PAPILIO_TSTRING(TypeParam, "\u00c4\u00c4");
        EXPECT_EQ(result, expected_str);
    }
}

TYPED_TEST(format_context_suite, format_to)
{
    using namespace papilio;

    using context_type = typename TestFixture::context_type;

    typename TestFixture::string_type result{};
    context_type ctx = TestFixture::create_context(result);

    using context_t = format_context_traits<context_type>;

    {
        context_t::format_to(ctx, PAPILIO_TSTRING_VIEW(TypeParam, "({:+})"), 1);

        const auto expected_str = PAPILIO_TSTRING(TypeParam, "(+1)");
        EXPECT_EQ(result, expected_str);
    }
}
