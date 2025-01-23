#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <set>
#include <ranges>
#include <papilio/format.hpp>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(ranges, sequence)
{
    std::vector<int> vec{1, 2, 3};

    EXPECT_EQ(PAPILIO_NS format("{}", vec), "[1, 2, 3]");
    EXPECT_EQ(PAPILIO_NS format("{:n}", vec), "1, 2, 3");
    EXPECT_EQ(PAPILIO_NS format("{:n:_^3}", vec), "_1_, _2_, _3_");

    EXPECT_EQ(PAPILIO_NS format(L"{}", vec), L"[1, 2, 3]");
    EXPECT_EQ(PAPILIO_NS format(L"{:n}", vec), L"1, 2, 3");
    EXPECT_EQ(PAPILIO_NS format(L"{:n:_^3}", vec), L"_1_, _2_, _3_");

    {
        {
            std::vector<const char*> chars_vec{
                "hello",
                "world"
            };

            EXPECT_EQ(PAPILIO_NS format("{}", chars_vec), R"(["hello", "world"])");
            EXPECT_EQ(PAPILIO_NS format("{::s}", chars_vec), "[hello, world]");
        }

        {
            std::vector<const wchar_t*> chars_vec{
                L"hello",
                L"world"
            };

            EXPECT_EQ(PAPILIO_NS format(L"{}", chars_vec), LR"(["hello", "world"])");
            EXPECT_EQ(PAPILIO_NS format(L"{::s}", chars_vec), L"[hello, world]");
        }
    }

    {
        std::vector<bool> bvec = {true, false, true};

        EXPECT_EQ(PAPILIO_NS format("{}", bvec), "[true, false, true]");
        EXPECT_EQ(PAPILIO_NS format("{::d}", bvec), "[1, 0, 1]");
        EXPECT_EQ(PAPILIO_NS format(L"{}", bvec), L"[true, false, true]");
        EXPECT_EQ(PAPILIO_NS format(L"{::d}", bvec), L"[1, 0, 1]");

        {
            std::locale loc = papilio_test::attach_yes_no<char>();
            EXPECT_EQ(PAPILIO_NS format(loc, "{::L}", bvec), "[yes, no, yes]");
            EXPECT_EQ(PAPILIO_NS format(loc, "{:n:L}", bvec), "yes, no, yes");
        }

        {
            std::locale loc = papilio_test::attach_yes_no<wchar_t>();
            EXPECT_EQ(PAPILIO_NS format(loc, L"{::L}", bvec), L"[yes, no, yes]");
            EXPECT_EQ(PAPILIO_NS format(loc, L"{:n:L}", bvec), L"yes, no, yes");
        }
    }

    // Workaround for libc++-15
#if !defined(PAPILIO_STDLIB_LIBCPP) || PAPILIO_STDLIB_LIBCPP >= 160000

    EXPECT_EQ(PAPILIO_NS format("{}", std::views::iota(1, 4)), "[1, 2, 3]");
    EXPECT_EQ(PAPILIO_NS format(L"{}", std::views::iota(1, 4)), L"[1, 2, 3]");

#endif
}

TEST(ranges, set)
{
    {
        std::set<int> s{1, 2, 3};

        EXPECT_EQ(PAPILIO_NS format("{}", s), "{1, 2, 3}");
        EXPECT_EQ(PAPILIO_NS format("{:n}", s), "1, 2, 3");

        EXPECT_EQ(PAPILIO_NS format(L"{}", s), L"{1, 2, 3}");
        EXPECT_EQ(PAPILIO_NS format(L"{:n}", s), L"1, 2, 3");
    }

    {
        std::set<bool> s{false, true};

        EXPECT_EQ(PAPILIO_NS format("{}", s), "{false, true}");
        EXPECT_EQ(PAPILIO_NS format("{:n}", s), "false, true");

        EXPECT_EQ(PAPILIO_NS format(L"{}", s), L"{false, true}");
        EXPECT_EQ(PAPILIO_NS format(L"{:n}", s), L"false, true");

        {
            std::locale loc = papilio_test::attach_yes_no<char>();
            EXPECT_EQ(PAPILIO_NS format(loc, "{::L}", s), "{no, yes}");
            EXPECT_EQ(PAPILIO_NS format(loc, "{:n:L}", s), "no, yes");
        }

        {
            std::locale loc = papilio_test::attach_yes_no<wchar_t>();
            EXPECT_EQ(PAPILIO_NS format(loc, L"{::L}", s), L"{no, yes}");
            EXPECT_EQ(PAPILIO_NS format(loc, L"{:n:L}", s), L"no, yes");
        }
    }
}

TEST(ranges, map)
{
    std::map<int, float> m{
        {1, 1.0f},
        {2, 2.0f},
        {3, 3.0f}
    };

    EXPECT_EQ(PAPILIO_NS format("{}", m), "{(1, 1), (2, 2), (3, 3)}");
    EXPECT_EQ(PAPILIO_NS format("{:n}", m), "(1, 1), (2, 2), (3, 3)");
    EXPECT_EQ(PAPILIO_NS format("{:m}", m), "{1: 1, 2: 2, 3: 3}");

    EXPECT_EQ(PAPILIO_NS format(L"{}", m), L"{(1, 1), (2, 2), (3, 3)}");
    EXPECT_EQ(PAPILIO_NS format(L"{:n}", m), L"(1, 1), (2, 2), (3, 3)");
    EXPECT_EQ(PAPILIO_NS format(L"{:m}", m), L"{1: 1, 2: 2, 3: 3}");
}

TEST(ranges, string_like)
{
    using namespace papilio;

    {
        static_assert(formattable<std::list<char>>);

        std::list<char> ls{'a', '"', 'b'};
        EXPECT_EQ(PAPILIO_NS format("{}", ls), R"(['a', '"', 'b'])");
        EXPECT_EQ(PAPILIO_NS format("{::c}", ls), R"([a, ", b])");
        EXPECT_EQ(PAPILIO_NS format("{:s}", ls), R"(a"b)");
        EXPECT_EQ(PAPILIO_NS format("{:?s}", ls), "\"a\\\"b\"");
    }

    {
        static_assert(formattable<std::list<wchar_t>, wchar_t>);

        std::list<wchar_t> ls{L'a', L'"', L'b'};
        EXPECT_EQ(PAPILIO_NS format(L"{}", ls), LR"(['a', '"', 'b'])");
        EXPECT_EQ(PAPILIO_NS format(L"{::c}", ls), LR"([a, ", b])");
        EXPECT_EQ(PAPILIO_NS format(L"{:s}", ls), LR"(a"b)");
        EXPECT_EQ(PAPILIO_NS format(L"{:?s}", ls), L"\"a\\\"b\"");
    }

    {
        static_assert(formattable<std::list<utf::codepoint>>);
        static_assert(formattable<std::list<utf::codepoint>, wchar_t>);

        std::list<utf::codepoint> ls{U'a'_cp, U'"'_cp, U'b'_cp};

        EXPECT_EQ(PAPILIO_NS format("{}", ls), R"(['a', '"', 'b'])");
        EXPECT_EQ(PAPILIO_NS format("{::c}", ls), R"([a, ", b])");
        EXPECT_EQ(PAPILIO_NS format("{:s}", ls), R"(a"b)");
        EXPECT_EQ(PAPILIO_NS format("{:?s}", ls), "\"a\\\"b\"");

        EXPECT_EQ(PAPILIO_NS format(L"{}", ls), LR"(['a', '"', 'b'])");
        EXPECT_EQ(PAPILIO_NS format(L"{::c}", ls), LR"([a, ", b])");
        EXPECT_EQ(PAPILIO_NS format(L"{:s}", ls), LR"(a"b)");
        EXPECT_EQ(PAPILIO_NS format(L"{:?s}", ls), L"\"a\\\"b\"");
    }
}

TEST(ranges, nested)
{
    using namespace papilio;

    {
        std::vector<std::vector<int>> v{
            {1, 2},
            {3, 4, 5},
            {6}
        };

        static_assert(formattable<decltype(v)>);

        EXPECT_EQ(PAPILIO_NS format("{}", v), "[[1, 2], [3, 4, 5], [6]]");
        EXPECT_EQ(PAPILIO_NS format(L"{}", v), L"[[1, 2], [3, 4, 5], [6]]");
    }
}
