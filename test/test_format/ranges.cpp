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

    std::vector<bool> bvec = {true, false, true};

    EXPECT_EQ(PAPILIO_NS format("{}", bvec), "[true, false, true]");
    EXPECT_EQ(PAPILIO_NS format("{::d}", bvec), "[1, 0, 1]");
    EXPECT_EQ(PAPILIO_NS format(L"{}", bvec), L"[true, false, true]");
    EXPECT_EQ(PAPILIO_NS format(L"{::d}", bvec), L"[1, 0, 1]");


    // Workaround for libc++-15
#if !defined(PAPILIO_STDLIB_LIBCPP) || PAPILIO_STDLIB_LIBCPP >= 160000

    EXPECT_EQ(PAPILIO_NS format("{}", std::views::iota(1, 4)), "[1, 2, 3]");
    EXPECT_EQ(PAPILIO_NS format(L"{}", std::views::iota(1, 4)), L"[1, 2, 3]");

#endif
}

TEST(ranges, set)
{
    std::set<int> s{1, 2, 3};

    EXPECT_EQ(PAPILIO_NS format("{}", s), "{1, 2, 3}");
    EXPECT_EQ(PAPILIO_NS format("{:n}", s), "1, 2, 3");

    EXPECT_EQ(PAPILIO_NS format(L"{}", s), L"{1, 2, 3}");
    EXPECT_EQ(PAPILIO_NS format(L"{:n}", s), L"1, 2, 3");
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

        std::list<char> vec{'a', '"', 'b'};
        EXPECT_EQ(PAPILIO_NS format("{}", vec), "[a, \", b]");
        EXPECT_EQ(PAPILIO_NS format("{:s}", vec), "a\"b");
        EXPECT_EQ(PAPILIO_NS format("{:?s}", vec), "a\\\"b");
    }

    {
        static_assert(formattable<std::list<wchar_t>, wchar_t>);

        std::list<wchar_t> vec{L'a', L'"', L'b'};
        EXPECT_EQ(PAPILIO_NS format(L"{}", vec), L"[a, \", b]");
        EXPECT_EQ(PAPILIO_NS format(L"{:s}", vec), L"a\"b");
        EXPECT_EQ(PAPILIO_NS format(L"{:?s}", vec), L"a\\\"b");
    }

    {
        static_assert(formattable<std::list<utf::codepoint>>);
        static_assert(formattable<std::list<utf::codepoint>, wchar_t>);

        std::list<utf::codepoint> vec{U'a'_cp, U'"'_cp, U'b'_cp};

        EXPECT_EQ(PAPILIO_NS format("{}", vec), "[a, \", b]");
        EXPECT_EQ(PAPILIO_NS format("{:s}", vec), "a\"b");
        EXPECT_EQ(PAPILIO_NS format("{:?s}", vec), "a\\\"b");

        EXPECT_EQ(PAPILIO_NS format(L"{}", vec), L"[a, \", b]");
        EXPECT_EQ(PAPILIO_NS format(L"{:s}", vec), L"a\"b");
        EXPECT_EQ(PAPILIO_NS format(L"{:?s}", vec), L"a\\\"b");
    }
}
