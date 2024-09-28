#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

template <typename T>
class int_formatter_suite : public ::testing::Test
{};

using int_types = ::testing::Types<
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
            std::to_string(i)
        ) << "val = "
          << val;

        EXPECT_EQ(
            PAPILIO_NS format(L"{}", val),
            std::to_wstring(val)
        ) << "val = "
          << val;
    }
}

TEST(int_formatter, extreme_value)
{
    using namespace papilio;

    {
        constexpr std::uint64_t val = std::numeric_limits<std::uint64_t>::max();
        EXPECT_EQ(PAPILIO_NS format("{}", val), std::to_string(val));
        EXPECT_EQ(PAPILIO_NS format(L"{}", val), std::to_wstring(val));

        {
            std::string buf;
            buf.reserve(64);
            PAPILIO_NS format_to(std::back_inserter(buf), "{:b}", val);
            EXPECT_EQ(buf.size(), 64);
            for(std::size_t i = 0; i < buf.size(); ++i)
                EXPECT_EQ(buf[i], '1') << "i = " << i;
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

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(3.14L)), "3.14");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(3.14L)), L"3.14");

    EXPECT_EQ(PAPILIO_NS format("{}", TypeParam(-3.14L)), "-3.14");
    EXPECT_EQ(PAPILIO_NS format(L"{}", TypeParam(-3.14L)), L"-3.14");
}

TYPED_TEST(float_formatter_suite, fill_and_align)
{
    if constexpr(std::is_same_v<TypeParam, float>)
    {
        const float pi = 3.14f;

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

TEST(fundamental_formatter, codepoint)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", 'a'), "a");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L'a'), L"a");

    EXPECT_EQ(PAPILIO_NS format("{:d}", 'a'), "97");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", L'a'), L"97");

    EXPECT_EQ(PAPILIO_NS format("{:?} {:?}", '\'', '"'), "\\' \"");
    EXPECT_EQ(PAPILIO_NS format(L"{:?} {:?}", '\'', '"'), L"\\' \"");
}

TEST(fundamental_formatter, string)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:?}", "hello\n\t\r"), "hello\\n\\t\\r");
    EXPECT_EQ(PAPILIO_NS format(L"{:?}", L"hello\n\t\r"), L"hello\\n\\t\\r");

    EXPECT_EQ(PAPILIO_NS format("{:s}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:s}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:.5}", "hello!"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:.5}", L"hello!"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:<8.5}", "hello!"), "hello   ");
    EXPECT_EQ(PAPILIO_NS format(L"{:<8.5}", L"hello!"), L"hello   ");
    EXPECT_EQ(PAPILIO_NS format("{:8.5}", "hello!"), "hello   ");
    EXPECT_EQ(PAPILIO_NS format(L"{:8.5}", L"hello!"), L"hello   ");

    EXPECT_EQ(PAPILIO_NS format("{:^8.5}", "hello!"), " hello  ");
    EXPECT_EQ(PAPILIO_NS format(L"{:^8.5}", L"hello!"), L" hello  ");
    EXPECT_EQ(PAPILIO_NS format("{:*^8.5}", "hello!"), "*hello**");
    EXPECT_EQ(PAPILIO_NS format(L"{:*^8.5}", L"hello!"), L"*hello**");

    EXPECT_EQ(PAPILIO_NS format("{:>8.5}", "hello!"), "   hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:>8.5}", L"hello!"), L"   hello");
    EXPECT_EQ(PAPILIO_NS format("{:*>8.5}", "hello!"), "***hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:*>8.5}", L"hello!"), L"***hello");
}

TEST(fundamental_formatter, bool)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", true), "true");
    EXPECT_EQ(PAPILIO_NS format(L"{}", true), L"true");
    EXPECT_EQ(PAPILIO_NS format("{}", false), "false");
    EXPECT_EQ(PAPILIO_NS format(L"{}", false), L"false");

    EXPECT_EQ(PAPILIO_NS format("{:d}", true), "1");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", true), L"1");
    EXPECT_EQ(PAPILIO_NS format("{:#x}", true), "0x1");
    EXPECT_EQ(PAPILIO_NS format(L"{:#x}", true), L"0x1");

    {
        std::locale loc = test_format::attach_yes_no<char>();
        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", true), "yes");
        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", false), "no");
    }

    {
        std::locale loc = test_format::attach_yes_no<wchar_t>();
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", true), L"yes");
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", false), L"no");
    }
}

TEST(fundamental_formatter, pointer)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", nullptr), "0x0");
    EXPECT_EQ(PAPILIO_NS format(L"{}", nullptr), L"0x0");
    EXPECT_EQ(PAPILIO_NS format("{:p}", nullptr), "0x0");
    EXPECT_EQ(PAPILIO_NS format(L"{:p}", nullptr), L"0x0");
    EXPECT_EQ(PAPILIO_NS format("{:P}", nullptr), "0X0");
    EXPECT_EQ(PAPILIO_NS format(L"{:P}", nullptr), L"0X0");

    {
        void* p = reinterpret_cast<void*>(0x7fff);
        const void* cp = p;

        EXPECT_EQ(PAPILIO_NS format("{:p}", p), "0x7fff");
        EXPECT_EQ(PAPILIO_NS format("{:p}", cp), "0x7fff");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", p), L"0x7fff");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", cp), L"0x7fff");

        EXPECT_EQ(PAPILIO_NS format("{:P}", p), "0X7FFF");
        EXPECT_EQ(PAPILIO_NS format("{:P}", cp), "0X7FFF");
        EXPECT_EQ(PAPILIO_NS format(L"{:P}", p), L"0X7FFF");
        EXPECT_EQ(PAPILIO_NS format(L"{:P}", cp), L"0X7FFF");
    }

    {
        char* p = nullptr;
        const char* cp = nullptr;

        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(p)), "0x0");
        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(cp)), "0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(p)), L"0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(cp)), L"0x0");
    }

    {
        std::unique_ptr<int> p;

        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(p)), "0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(p)), L"0x0");
    }

    {
        std::shared_ptr<int> p;

        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(p)), "0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(p)), L"0x0");
    }
}

TEST(fundamental_formatter, magic_enum)
{
    using namespace papilio;

    enum animal
    {
        cat = 1,
        dog
    };

#ifdef PAPILIO_HAS_ENUM_NAME

    EXPECT_EQ(PAPILIO_NS format("{}", cat), "cat");
    EXPECT_EQ(PAPILIO_NS format("{}", dog), "dog");
    EXPECT_EQ(PAPILIO_NS format("{:>5s}", dog), "  dog");

    EXPECT_EQ(PAPILIO_NS format(L"{}", cat), L"cat");
    EXPECT_EQ(PAPILIO_NS format(L"{}", dog), L"dog");
    EXPECT_EQ(PAPILIO_NS format(L"{:>5s}", dog), L"  dog");

#endif

    EXPECT_EQ(PAPILIO_NS format("{:d}", cat), "1");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", cat), L"1");
    EXPECT_EQ(PAPILIO_NS format("{:#x}", cat), "0x1");
    EXPECT_EQ(PAPILIO_NS format(L"{:#x}", cat), L"0x1");
}
