#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include "test_format.hpp"

namespace test_format
{
template <std::integral T>
void test_int_formatter()
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", T(0)), "0");
    EXPECT_EQ(PAPILIO_NS format(L"{}", T(0)), L"0");

    EXPECT_EQ(PAPILIO_NS format("{:6}", T(42)), "    42");
    EXPECT_EQ(PAPILIO_NS format(L"{:6}", T(42)), L"    42");

    EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", T(1)), "1,+1,1, 1");
    EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", T(1)), L"1,+1,1, 1");
    if constexpr(std::is_signed_v<T>)
    {
        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", T(-1)), "-1,-1,-1,-1");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", T(-1)), L"-1,-1,-1,-1");
    }

    EXPECT_EQ(PAPILIO_NS format("{:+06d}", T(42)), "+00042");
    EXPECT_EQ(PAPILIO_NS format(L"{:+06d}", T(42)), L"+00042");
    EXPECT_EQ(PAPILIO_NS format("{:#06x}", T(0xa)), "0x000a");
    EXPECT_EQ(PAPILIO_NS format(L"{:#06x}", T(0xa)), L"0x000a");
    if constexpr(std::is_signed_v<T>)
    {
        EXPECT_EQ(PAPILIO_NS format("{:<06}", T(-42)), "-42   ");
        EXPECT_EQ(PAPILIO_NS format(L"{:<06}", T(-42)), L"-42   ");
    }

    EXPECT_EQ(PAPILIO_NS format("{:{}d}", T(42), 4), "  42");
    EXPECT_EQ(PAPILIO_NS format(L"{:{}d}", T(42), 4), L"  42");

    EXPECT_EQ(PAPILIO_NS format("{:d>6}", T(42)), "dddd42");
    EXPECT_EQ(PAPILIO_NS format(L"{:d>6}", T(42)), L"dddd42");

    EXPECT_EQ(PAPILIO_NS format("{:^5c}", T(97)), "  a  ");
    EXPECT_EQ(PAPILIO_NS format(L"{:^5c}", T(97)), L"  a  ");
}

template <std::floating_point T>
void test_float_formatter()
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", T(3.14L)), "3.14");
    EXPECT_EQ(PAPILIO_NS format(L"{}", T(3.14L)), L"3.14");

    {
        const T inf = std::numeric_limits<T>::infinity();

        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", inf), "inf,+inf,inf, inf");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", inf), L"inf,+inf,inf, inf");

        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", -inf), "-inf,-inf,-inf,-inf");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", -inf), L"-inf,-inf,-inf,-inf");
    }

    {
        const T nan = std::numeric_limits<T>::quiet_NaN();

        EXPECT_EQ(PAPILIO_NS format("{}", nan), "nan");
        EXPECT_EQ(PAPILIO_NS format(L"{}", nan), L"nan");
    }
}
} // namespace test_format

TEST(formatter, int)
{
    using test_format::test_int_formatter;

    test_int_formatter<int>();
    test_int_formatter<unsigned int>();
    test_int_formatter<long long int>();
    test_int_formatter<unsigned long long int>();
}

TEST(formatter, float)
{
    using test_format::test_float_formatter;

    test_float_formatter<float>();
    test_float_formatter<double>();
    test_float_formatter<long double>();

    using namespace papilio;

    const float pi = 3.14f;

    EXPECT_EQ(PAPILIO_NS format("{:10f}", pi), "  3.140000");
    EXPECT_EQ(PAPILIO_NS format(L"{:10f}", pi), L"  3.140000");

    EXPECT_EQ(PAPILIO_NS format("{:.5f}", pi), "3.14000");
    EXPECT_EQ(PAPILIO_NS format(L"{:.5f}", pi), L"3.14000");

    EXPECT_EQ(PAPILIO_NS format("{:10.5f}", pi), "   3.14000");
    EXPECT_EQ(PAPILIO_NS format(L"{:10.5f}", pi), L"   3.14000");
}

TEST(formatter, codepoint)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", 'a'), "a");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L'a'), L"a");

    EXPECT_EQ(PAPILIO_NS format("{:d}", 'a'), "97");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", L'a'), L"97");
}

TEST(formatter, string)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:s}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:s}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:.5}", "hello!"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:.5}", L"hello!"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:^8.5}", "hello!"), " hello  ");
    EXPECT_EQ(PAPILIO_NS format(L"{:^8.5}", L"hello!"), L" hello  ");
}

TEST(formatter, bool)
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
        auto loc = test_format::attach_yes_no<char>();
        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", true), "yes");
        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", false), "no");
    }

    {
        auto loc = test_format::attach_yes_no<wchar_t>();
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", true), L"yes");
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", false), L"no");
    }
}

TEST(formatter, pointer)
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

TEST(format, magic_enum)
{
    using namespace papilio;

    enum animal
    {
        cat = 1,
        dog
    };

    EXPECT_EQ(PAPILIO_NS format("{}", cat), "cat");
    EXPECT_EQ(PAPILIO_NS format("{}", dog), "dog");
    EXPECT_EQ(PAPILIO_NS format("{:d}", cat), "1");

    EXPECT_EQ(PAPILIO_NS format(L"{}", cat), L"cat");
    EXPECT_EQ(PAPILIO_NS format(L"{}", dog), L"dog");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", cat), L"1");
}
