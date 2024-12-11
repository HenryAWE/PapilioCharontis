#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <random>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

template <typename T>
class int_formatter_suite : public ::testing::Test
{};

using int_types = ::testing::Types<
    signed char,
    unsigned char,
    short,
    unsigned short,
    int,
    unsigned int,
    long long int,
    unsigned long long int>;
TYPED_TEST_SUITE(int_formatter_suite, int_types);

TYPED_TEST(int_formatter_suite, basic)
{
    using namespace papilio;

    for(int i : {0, 1, 2, 7, 8, 15, 16})
    {
        TypeParam val = static_cast<TypeParam>(i);

        EXPECT_EQ(
            PAPILIO_NS format("{}", val),
            std::to_string(val)
        ) << "val = "
          << val;

        EXPECT_EQ(
            PAPILIO_NS format(L"{}", val),
            std::to_wstring(val)
        ) << "val = "
          << val;
    }
}

TYPED_TEST(int_formatter_suite, random)
{
    if constexpr(sizeof(TypeParam) >= sizeof(short))
    {
        using namespace papilio;

        std::random_device rd;
        auto seed = rd();

        SCOPED_TRACE("seed = " + std::to_string(seed));
        std::mt19937 gen(seed);

        std::uniform_int_distribution<TypeParam> dist(
            std::numeric_limits<TypeParam>::min(),
            std::numeric_limits<TypeParam>::max()
        );

        using unsigned_t = std::make_unsigned_t<TypeParam>;
        constexpr unsigned_t loop_count = std::min(
            unsigned_t(std::numeric_limits<unsigned_t>::max() / 20),
            unsigned_t(4096)
        );
        for(unsigned_t i = 0; i < loop_count; ++i)
        {
            TypeParam val = dist(gen);
            EXPECT_EQ(PAPILIO_NS format("{}", val), std::to_string(val))
                << "val=" << val;
        }
    }
}

TYPED_TEST(int_formatter_suite, extreme_value)
{
    using namespace papilio;

    if constexpr(std::is_unsigned_v<TypeParam>)
    {
        constexpr std::uint64_t val = std::numeric_limits<TypeParam>::max();
        EXPECT_EQ(PAPILIO_NS format("{}", val), std::to_string(val));
        EXPECT_EQ(PAPILIO_NS format(L"{}", val), std::to_wstring(val));


        std::string buf;
        constexpr std::size_t bits = sizeof(TypeParam) * 8;
        buf.reserve(bits);
        PAPILIO_NS format_to(std::back_inserter(buf), "{:b}", val);
        EXPECT_EQ(buf.size(), bits);
        for(std::size_t i = 0; i < buf.size(); ++i)
            EXPECT_EQ(buf[i], '1') << "i = " << i;
    }
    else
    {
        {
            constexpr std::int64_t val = std::numeric_limits<TypeParam>::max();
            EXPECT_EQ(PAPILIO_NS format("{}", val), std::to_string(val));
            EXPECT_EQ(PAPILIO_NS format(L"{}", val), std::to_wstring(val));

            std::string buf;
            constexpr std::size_t bits = sizeof(TypeParam) * 8;
            buf.reserve(bits - 1);
            PAPILIO_NS format_to(std::back_inserter(buf), "{:b}", val);
            EXPECT_EQ(buf.size(), bits - 1);
            for(std::size_t i = 0; i < buf.size(); ++i)
                EXPECT_EQ(buf[i], '1') << "i = " << i;
        }

        {
            constexpr std::int64_t val = std::numeric_limits<TypeParam>::min();
            EXPECT_EQ(PAPILIO_NS format("{}", val), std::to_string(val));
            EXPECT_EQ(PAPILIO_NS format(L"{}", val), std::to_wstring(val));

            std::string buf;
            constexpr std::size_t bits = sizeof(TypeParam) * 8;
            buf.reserve(bits + 1);
            PAPILIO_NS format_to(std::back_inserter(buf), "{:b}", val);
            EXPECT_EQ(buf.size(), bits + 1);
            EXPECT_EQ(buf[0], '-');
            EXPECT_EQ(buf[1], '1');
            for(std::size_t i = 2; i < buf.size(); ++i)
                EXPECT_EQ(buf[i], '0') << "i = " << i;
        }
    }
}

TYPED_TEST(int_formatter_suite, fill_and_align)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{:6}", TypeParam(42)), "    42");
    EXPECT_EQ(PAPILIO_NS format(L"{:6}", TypeParam(42)), L"    42");

    EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", TypeParam(1)), "1,+1,1, 1");
    EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", TypeParam(1)), L"1,+1,1, 1");
    if constexpr(std::is_signed_v<TypeParam>)
    {
        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", TypeParam(-1)), "-1,-1,-1,-1");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", TypeParam(-1)), L"-1,-1,-1,-1");
    }

    EXPECT_EQ(PAPILIO_NS format("{:+06d}", TypeParam(42)), "+00042");
    EXPECT_EQ(PAPILIO_NS format(L"{:+06d}", TypeParam(42)), L"+00042");
    EXPECT_EQ(PAPILIO_NS format("{:#06x}", TypeParam(0xa)), "0x000a");
    EXPECT_EQ(PAPILIO_NS format(L"{:#06x}", TypeParam(0xa)), L"0x000a");
    if constexpr(std::is_signed_v<TypeParam>)
    {
        EXPECT_EQ(PAPILIO_NS format("{:<06}", TypeParam(-42)), "-42   ");
        EXPECT_EQ(PAPILIO_NS format(L"{:<06}", TypeParam(-42)), L"-42   ");
    }

    EXPECT_EQ(PAPILIO_NS format("{:{}d}", TypeParam(42), 4), "  42");
    EXPECT_EQ(PAPILIO_NS format(L"{:{}d}", TypeParam(42), 4), L"  42");

    EXPECT_EQ(PAPILIO_NS format("{:d>6}", TypeParam(42)), "dddd42");
    EXPECT_EQ(PAPILIO_NS format(L"{:d>6}", TypeParam(42)), L"dddd42");

    EXPECT_EQ(PAPILIO_NS format("{:^5c}", TypeParam(97)), "  a  ");
    EXPECT_EQ(PAPILIO_NS format(L"{:^5c}", TypeParam(97)), L"  a  ");
}

namespace test_format
{
template <typename CharT>
class my_int_sep : public std::numpunct<CharT>
{
public:
    using char_type = typename std::numpunct<CharT>::char_type;

protected:
    char_type do_thousands_sep() const override
    {
        return char_type('.');
    }

    std::string do_grouping() const override
    {
        return std::string("\1\2\3", 3);
    }
};

template <typename CharT = char>
std::locale attach_my_int_sep()
{
    return std::locale(std::locale::classic(), new my_int_sep<CharT>());
}
} // namespace test_format

TEST(int_formatter, locale)
{
    using namespace papilio;

    {
        std::locale loc = test_format::attach_my_int_sep();

        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", 123456789), "123.456.78.9");

        EXPECT_EQ(PAPILIO_NS format(loc, "{:012}", 123456789), "000123456789");
        EXPECT_EQ(PAPILIO_NS format(loc, "{:012L}", 123456789), "000.123.456.78.9");

        EXPECT_EQ(
            PAPILIO_NS format(loc, "{:L}", std::numeric_limits<std::uint64_t>::max()),
            "18.446.744.073.709.551.61.5"
        );
    }

    {
        std::locale loc = test_format::attach_my_int_sep<wchar_t>();

        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", 123456789), L"123.456.78.9");

        EXPECT_EQ(PAPILIO_NS format(loc, L"{:012}", 123456789), L"000123456789");
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:012L}", 123456789), L"000.123.456.78.9");

        EXPECT_EQ(
            PAPILIO_NS format(loc, L"{:L}", std::numeric_limits<std::uint64_t>::max()),
            L"18.446.744.073.709.551.61.5"
        );
    }
}
