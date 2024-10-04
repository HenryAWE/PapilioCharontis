#include <gtest/gtest.h>
#include <vector>
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

    EXPECT_EQ(PAPILIO_NS format("{}", std::views::iota(1, 4)), "[1, 2, 3]");
    EXPECT_EQ(PAPILIO_NS format(L"{}", std::views::iota(1, 4)), L"[1, 2, 3]");
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
