#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <vector>
#include <iostream>
#include "test_format.hpp"

TYPED_TEST(format_suite, plain_text)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

    {
        string_view_type empty_fmt{};

        EXPECT_EQ(
            PAPILIO_NS format(empty_fmt),
            string_view_type()
        );
    }

    {
        string_view_type plain_text = PAPILIO_TSTRING_VIEW(TypeParam, "plain text");

        EXPECT_EQ(
            PAPILIO_NS format(plain_text),
            plain_text
        );
    }

    {
        string_view_type esc_seq = PAPILIO_TSTRING_VIEW(TypeParam, "{{plain text}}");
        string_view_type expected_str = PAPILIO_TSTRING_VIEW(TypeParam, "{plain text}");

        EXPECT_EQ(
            PAPILIO_NS format(esc_seq),
            expected_str
        );
    }
}

TYPED_TEST(format_suite, format_to)
{
    using namespace papilio;

    {
        const auto vec_str = PAPILIO_TSTRING(TypeParam, "vec");

        std::vector<TypeParam> result;
        auto it = PAPILIO_NS format_to(
            std::back_inserter(result),
            vec_str
        );
        *it = '\0';

        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), vec_str);
    }

    {
        std::locale loc = test_format::attach_yes_no<TypeParam>();

        std::vector<TypeParam> result;
        auto it = PAPILIO_NS format_to(
            std::back_inserter(result),
            loc,
            PAPILIO_TSTRING_VIEW(TypeParam, "{:L}"),
            true
        );
        *it = '\0';

        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(
            result.data(),
            test_format::yes_no_numpunct<TypeParam>::yes_string
        );
    }
}

TYPED_TEST(format_suite, formatted_size)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

    EXPECT_EQ(PAPILIO_NS formatted_size(string_view_type()), 0);

    {
        string_view_type fmt = PAPILIO_TSTRING_VIEW(TypeParam, "hello");
        EXPECT_EQ(PAPILIO_NS formatted_size(fmt), 5);
    }

    {
        string_view_type fmt = PAPILIO_TSTRING_VIEW(TypeParam, "{{hello}}");
        EXPECT_EQ(PAPILIO_NS formatted_size(fmt), 7); // Size of "{hello}"
    }

    {
        std::locale loc = test_format::attach_yes_no<TypeParam>();

        string_view_type fmt = PAPILIO_TSTRING_VIEW(TypeParam, "{:L}");
        EXPECT_EQ(PAPILIO_NS formatted_size(loc, fmt, true), 3); // Size of "yes"
    }
}

TYPED_TEST(format_suite, format_to_n)
{
    using namespace papilio;

    using string_type = typename TestFixture::string_type;
    using string_view_type = typename TestFixture::string_view_type;

    {
        string_type str{};
        str.resize(5);
        auto result = PAPILIO_NS format_to_n(
            str.begin(),
            str.size(),
            PAPILIO_TSTRING_VIEW(TypeParam, "hello world")
        );

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 5);

        const auto expected_str = PAPILIO_TSTRING_VIEW(TypeParam, "hello");
        EXPECT_EQ(str, expected_str);
    }

    {
        std::locale loc = test_format::attach_yes_no<TypeParam>();

        string_type str{};
        str.resize(4);
        auto result = PAPILIO_NS format_to_n(
            str.begin(),
            str.size(),
            loc,
            PAPILIO_TSTRING_VIEW(TypeParam, "{:L}!!"),
            true
        );

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 4);

        const auto expected_str = PAPILIO_TSTRING_VIEW(TypeParam, "yes!");
        EXPECT_EQ(str, expected_str);
    }
}

TYPED_TEST(format_suite, exception)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

    const string_view_type bad_fmts[]{
        PAPILIO_TSTRING_VIEW(TypeParam, "{"),
        PAPILIO_TSTRING_VIEW(TypeParam, "}")
    };

    for(string_view_type f : bad_fmts)
    {
        EXPECT_THROW(
            (void)PAPILIO_NS format(f),
            papilio::format_error
        ) << "f = "
          << std::quoted(utf::basic_string_ref<TypeParam>(f).to_string());
    }
}
