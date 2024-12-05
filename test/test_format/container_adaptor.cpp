#include <gtest/gtest.h>
#include <stack>
#include <papilio/format.hpp>
#include <papilio/formatter/container_adaptor.hpp>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(container_adaptor, stack)
{
    using namespace papilio;

    {
        static_assert(formattable<std::stack<int>>);

        std::stack<int> s;
        s.push(1);
        s.push(2);

        EXPECT_EQ(PAPILIO_NS format("{}", s), "[1, 2]");
        EXPECT_EQ(PAPILIO_NS format(L"{}", s), L"[1, 2]");
    }
}

TEST(container_adaptor, queue)
{
    using namespace papilio;

    {
        static_assert(formattable<std::queue<int>>);

        std::queue<int> s;
        s.push(1);
        s.push(2);

        EXPECT_EQ(PAPILIO_NS format("{}", s), "[1, 2]");
        EXPECT_EQ(PAPILIO_NS format(L"{}", s), L"[1, 2]");
    }
}

TEST(container_adaptor, priority_queue)
{
    using namespace papilio;

    {
        static_assert(formattable<std::priority_queue<int>>);

        std::priority_queue<int> s;
        s.push(1);
        s.push(2);

        EXPECT_EQ(PAPILIO_NS format("{}", s), "[2, 1]");
        EXPECT_EQ(PAPILIO_NS format(L"{}", s), L"[2, 1]");
    }
}
