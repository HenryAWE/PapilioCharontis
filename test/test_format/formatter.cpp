#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <random>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(fundamental_formatter, codepoint)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", 'a'), "a");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L'a'), L"a");

    EXPECT_EQ(PAPILIO_NS format("{:d}", 'a'), "97");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", L'a'), L"97");

    EXPECT_EQ(PAPILIO_NS format("{:?} {:?}", '\'', '"'), R"('\'' '"')");
    EXPECT_EQ(PAPILIO_NS format(L"{:?} {:?}", '\'', '"'), LR"('\'' '"')");
}

TEST(fundamental_formatter, string)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:?}", "hello\n\t\r"), R"("hello\n\t\r")");
    EXPECT_EQ(PAPILIO_NS format(L"{:?}", L"hello\n\t\r"), LR"("hello\n\t\r")");

    EXPECT_EQ(PAPILIO_NS format("{:?}", std::string("\0 \n \t \x02 \x1b", 9)), R"("\u{0} \n \t \u{2} \u{1b}")");
    EXPECT_EQ(PAPILIO_NS format(L"{:?}", std::wstring(L"\0 \n \t \x02 \x1b", 9)), LR"("\u{0} \n \t \u{2} \u{1b}")");

    EXPECT_EQ(PAPILIO_NS format("{:?}", std::string("\xc3\x28", 2)), "\"\\x{c3}(\"");

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

    {
        std::vector<bool> bvec = {false, true};

        static_assert(formattable<std::vector<bool>::reference>);

        EXPECT_EQ(PAPILIO_NS format("{}", bvec[1]), "true");
        EXPECT_EQ(PAPILIO_NS format(L"{}", bvec[1]), L"true");
        EXPECT_EQ(PAPILIO_NS format("{}", bvec[0]), "false");
        EXPECT_EQ(PAPILIO_NS format(L"{}", bvec[0]), L"false");

        EXPECT_EQ(PAPILIO_NS format("{:d}", bvec[1]), "1");
        EXPECT_EQ(PAPILIO_NS format(L"{:d}", bvec[1]), L"1");
        EXPECT_EQ(PAPILIO_NS format("{:#x}", bvec[1]), "0x1");
        EXPECT_EQ(PAPILIO_NS format(L"{:#x}", bvec[1]), L"0x1");

        {
            std::locale loc = test_format::attach_yes_no<char>();
            EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", bvec[1]), "yes");
            EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", bvec[0]), "no");
        }

        {
            std::locale loc = test_format::attach_yes_no<wchar_t>();
            EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", bvec[1]), L"yes");
            EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", bvec[0]), L"no");
        }
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
        std::unique_ptr<char> p;
        const std::unique_ptr<char> cp;

        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(p)), "0x0");
        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(cp)), "0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(p)), L"0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(cp)), L"0x0");
    }

    {
        std::shared_ptr<char> p;
        const std::shared_ptr<char> cp;

        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(p)), "0x0");
        EXPECT_EQ(PAPILIO_NS format("{:p}", PAPILIO_NS ptr(cp)), "0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(p)), L"0x0");
        EXPECT_EQ(PAPILIO_NS format(L"{:p}", PAPILIO_NS ptr(cp)), L"0x0");
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
