#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <random>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

template <typename T>
class float_formatter_suite : public ::testing::Test
{
public:
    static T create_inf() noexcept
    {
        return std::numeric_limits<T>::infinity();
    }

    static T create_nan() noexcept
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
};

using float_types = ::testing::Types<float, double, long double>;
TYPED_TEST_SUITE(float_formatter_suite, float_types);

TYPED_TEST(float_formatter_suite, basic)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(0.0L)), "0");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(0.0L)), L"0");

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(-0.0L)), "-0");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(-0.0L)), L"-0");

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(42)), "42");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(42)), L"42");

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(3.14L)), "3.14");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(3.14L)), L"3.14");

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(-3.14L)), "-3.14");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(-3.14L)), L"-3.14");

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(1.0e10L)), "1e+10");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(1.0e10L)), L"1e+10");
}

TYPED_TEST(float_formatter_suite, inf_and_nan)
{
    EXPECT_EQ(PAPILIO_NS format("{}", TestFixture::create_inf()), "inf");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TestFixture::create_inf()), L"inf");
    EXPECT_EQ(PAPILIO_NS format("{:g}", TestFixture::create_inf()), "inf");
    EXPECT_EQ(PAPILIO_NS format(L"{:g}", TestFixture::create_inf()), L"inf");
    EXPECT_EQ(PAPILIO_NS format("{:f}", TestFixture::create_inf()), "inf");
    EXPECT_EQ(PAPILIO_NS format(L"{:f}", TestFixture::create_inf()), L"inf");
    EXPECT_EQ(PAPILIO_NS format("{:e}", TestFixture::create_inf()), "inf");
    EXPECT_EQ(PAPILIO_NS format(L"{:e}", TestFixture::create_inf()), L"inf");

    EXPECT_EQ(PAPILIO_NS format("{:G}", TestFixture::create_inf()), "INF");
    EXPECT_EQ(PAPILIO_NS format(L"{:G}", TestFixture::create_inf()), L"INF");
    EXPECT_EQ(PAPILIO_NS format("{:F}", TestFixture::create_inf()), "INF");
    EXPECT_EQ(PAPILIO_NS format(L"{:F}", TestFixture::create_inf()), L"INF");
    EXPECT_EQ(PAPILIO_NS format("{:E}", TestFixture::create_inf()), "INF");
    EXPECT_EQ(PAPILIO_NS format(L"{:E}", TestFixture::create_inf()), L"INF");

    EXPECT_EQ(PAPILIO_NS format("{}", TestFixture::create_nan()), "nan");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TestFixture::create_nan()), L"nan");
    EXPECT_EQ(PAPILIO_NS format("{:g}", TestFixture::create_nan()), "nan");
    EXPECT_EQ(PAPILIO_NS format(L"{:g}", TestFixture::create_nan()), L"nan");
    EXPECT_EQ(PAPILIO_NS format("{:f}", TestFixture::create_nan()), "nan");
    EXPECT_EQ(PAPILIO_NS format(L"{:f}", TestFixture::create_nan()), L"nan");
    EXPECT_EQ(PAPILIO_NS format("{:e}", TestFixture::create_nan()), "nan");
    EXPECT_EQ(PAPILIO_NS format(L"{:e}", TestFixture::create_nan()), L"nan");

    EXPECT_EQ(PAPILIO_NS format("{:G}", TestFixture::create_nan()), "NAN");
    EXPECT_EQ(PAPILIO_NS format(L"{:G}", TestFixture::create_nan()), L"NAN");
    EXPECT_EQ(PAPILIO_NS format("{:F}", TestFixture::create_nan()), "NAN");
    EXPECT_EQ(PAPILIO_NS format(L"{:F}", TestFixture::create_nan()), L"NAN");
    EXPECT_EQ(PAPILIO_NS format("{:E}", TestFixture::create_nan()), "NAN");
    EXPECT_EQ(PAPILIO_NS format(L"{:E}", TestFixture::create_nan()), L"NAN");
}

TYPED_TEST(float_formatter_suite, scientific)
{
    using namespace papilio;

    {
        TypeParam val = TypeParam(1.0e-4L);

        EXPECT_EQ(PAPILIO_NS format("{:e}", val), "1.000000e-04");
        EXPECT_EQ(PAPILIO_NS format(L"{:e}", val), L"1.000000e-04");

        EXPECT_EQ(PAPILIO_NS format("{:E}", val), "1.000000E-04");
        EXPECT_EQ(PAPILIO_NS format(L"{:E}", val), L"1.000000E-04");

        EXPECT_EQ(PAPILIO_NS format("{:.2e}", val), "1.00e-04");
        EXPECT_EQ(PAPILIO_NS format(L"{:.2e}", val), L"1.00e-04");
    }

    {
        TypeParam val = TypeParam(1.0e10L);

        EXPECT_EQ(PAPILIO_NS format("{:e}", val), "1.000000e+10");
        EXPECT_EQ(PAPILIO_NS format(L"{:e}", val), L"1.000000e+10");

        EXPECT_EQ(PAPILIO_NS format("{:E}", val), "1.000000E+10");
        EXPECT_EQ(PAPILIO_NS format(L"{:E}", val), L"1.000000E+10");

        EXPECT_EQ(PAPILIO_NS format("{:.2e}", val), "1.00e+10");
        EXPECT_EQ(PAPILIO_NS format(L"{:.2e}", val), L"1.00e+10");
    }
}

TYPED_TEST(float_formatter_suite, hex)
{
    using namespace papilio;

    if constexpr(!std::is_same_v<TypeParam, long double>)
    {
        TypeParam hex_pi = TypeParam(0x1.921fb6p1);

        EXPECT_EQ(PAPILIO_NS format("{:a}", hex_pi), "1.921fb6p+1");
        EXPECT_EQ(PAPILIO_NS format(L"{:a}", hex_pi), L"1.921fb6p+1");

        EXPECT_EQ(PAPILIO_NS format("{:A}", hex_pi), "1.921FB6P+1");
        EXPECT_EQ(PAPILIO_NS format(L"{:A}", hex_pi), L"1.921FB6P+1");
    }
}

TYPED_TEST(float_formatter_suite, fill_and_align)
{
    {
        const TypeParam pi = TypeParam(3.14L);

        EXPECT_EQ(PAPILIO_NS format("{:10f}", pi), "  3.140000");
        EXPECT_EQ(PAPILIO_NS format(L"{:10f}", pi), L"  3.140000");

        EXPECT_EQ(PAPILIO_NS format("{:.5f}", pi), "3.14000");
        EXPECT_EQ(PAPILIO_NS format(L"{:.5f}", pi), L"3.14000");

        EXPECT_EQ(PAPILIO_NS format("{:10.5f}", pi), "   3.14000");
        EXPECT_EQ(PAPILIO_NS format(L"{:10.5f}", pi), L"   3.14000");
    }

    {
        const TypeParam inf = TestFixture::create_inf();

        EXPECT_EQ(
            PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", inf),
            "inf,+inf,inf, inf"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", inf),
            L"inf,+inf,inf, inf"
        );

        EXPECT_EQ(
            PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", -inf),
            "-inf,-inf,-inf,-inf"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", -inf),
            L"-inf,-inf,-inf,-inf"
        );
    }

    {
        const TypeParam nan = TestFixture::create_nan();

        EXPECT_EQ(
            PAPILIO_NS format("{}", nan),
            "nan"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{}", nan),
            L"nan"
        );
    }
}

namespace test_format
{
template <typename CharT>
class my_float_numpunct : public std::numpunct<CharT>
{
public:
    using char_type = typename std::numpunct<CharT>::char_type;

protected:
    char_type do_thousands_sep() const override
    {
        return char_type('.');
    }

    char_type do_decimal_point() const override
    {
        return char_type(',');
    }

    std::string do_grouping() const override
    {
        return std::string("\1\2\3", 3);
    }
};

template <typename CharT = char>
std::locale attach_my_float_sep()
{
    return std::locale(std::locale::classic(), new my_float_numpunct<CharT>());
}
} // namespace test_format

TEST(float_formatter, locale)
{
    {
        std::locale loc = test_format::attach_my_float_sep();

        EXPECT_EQ(PAPILIO_NS format(loc, "{:f}", 123456789.123456789), "123456789.123457");
        EXPECT_EQ(PAPILIO_NS format(loc, "{:Lf}", 123456789.123456789), "123.456.78.9,123457");
    }

    {
        std::locale loc = test_format::attach_my_float_sep<wchar_t>();

        EXPECT_EQ(PAPILIO_NS format(loc, L"{:f}", 123456789.123456789), L"123456789.123457");
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:Lf}", 123456789.123456789), L"123.456.78.9,123457");
    }
}
