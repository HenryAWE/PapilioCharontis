#include <gtest/gtest.h>
#include <papilio/container.hpp>
#include <papilio_test/setup.hpp>

TEST(fixed_vector, push_back_int)
{
    using papilio::fixed_vector;

    fixed_vector<int, 2> fv;
    EXPECT_EQ(fv.size(), 0);
    EXPECT_TRUE(fv.empty());

    EXPECT_THROW(fv.at(0), std::out_of_range);
    EXPECT_THROW(std::as_const(fv).at(0), std::out_of_range);

    fv.push_back(0);
    EXPECT_EQ(fv.size(), 1);
    EXPECT_FALSE(fv.empty());
    EXPECT_EQ(fv[0], 0);
    EXPECT_EQ(std::as_const(fv)[0], 0);
    EXPECT_EQ(fv.at(0), 0);
    EXPECT_EQ(std::as_const(fv).at(0), 0);

    EXPECT_EQ(fv.front(), 0);
    EXPECT_EQ(std::as_const(fv).front(), 0);
    EXPECT_EQ(fv.back(), 0);
    EXPECT_EQ(std::as_const(fv).back(), 0);

    fv.push_back(1);
    EXPECT_EQ(fv.size(), 2);
    EXPECT_FALSE(fv.empty());
    EXPECT_EQ(fv[1], 1);
    EXPECT_EQ(std::as_const(fv)[1], 1);
    EXPECT_EQ(fv.at(1), 1);
    EXPECT_EQ(std::as_const(fv).at(1), 1);

    EXPECT_EQ(fv.front(), 0);
    EXPECT_EQ(std::as_const(fv).front(), 0);
    EXPECT_EQ(fv.back(), 1);
    EXPECT_EQ(std::as_const(fv).back(), 1);

    EXPECT_THROW(fv.push_back(2), std::length_error);
}

TEST(fixed_vector, push_back_string)
{
    using papilio::fixed_vector;

    fixed_vector<std::string, 2> fv;
    EXPECT_EQ(fv.size(), 0);
    EXPECT_TRUE(fv.empty());
    EXPECT_EQ(fv.capacity(), 2);
    EXPECT_EQ(fv.max_size(), 2);
    static_assert(decltype(fv)::max_size() == 2);

    EXPECT_THROW(fv.at(0), std::out_of_range);
    EXPECT_THROW(std::as_const(fv).at(0), std::out_of_range);

    fv.push_back("first");
    EXPECT_EQ(fv.size(), 1);
    EXPECT_FALSE(fv.empty());
    EXPECT_EQ(fv[0], "first");
    EXPECT_EQ(std::as_const(fv)[0], "first");
    EXPECT_EQ(fv.at(0), "first");
    EXPECT_EQ(std::as_const(fv).at(0), "first");

    EXPECT_EQ(fv.front(), "first");
    EXPECT_EQ(std::as_const(fv).front(), "first");
    EXPECT_EQ(fv.back(), "first");
    EXPECT_EQ(std::as_const(fv).back(), "first");

    fv.push_back("second");
    EXPECT_EQ(fv.size(), 2);
    EXPECT_FALSE(fv.empty());
    EXPECT_EQ(fv[1], "second");
    EXPECT_EQ(std::as_const(fv)[1], "second");
    EXPECT_EQ(fv.at(1), "second");
    EXPECT_EQ(std::as_const(fv).at(1), "second");

    EXPECT_EQ(fv.front(), "first");
    EXPECT_EQ(std::as_const(fv).front(), "first");
    EXPECT_EQ(fv.back(), "second");
    EXPECT_EQ(std::as_const(fv).back(), "second");

    EXPECT_THROW(fv.push_back("third"), std::length_error);
}

TEST(fixed_vector, insert)
{
    using papilio::fixed_vector;

    fixed_vector<std::string, 4> fv;

    EXPECT_EQ(fv.insert(fv.end(), "world"), fv.begin());
    EXPECT_EQ(fv.back(), "world");
    EXPECT_EQ(fv.size(), 1);

    EXPECT_EQ(fv.insert(fv.begin(), "hello"), fv.begin());
    EXPECT_EQ(fv.front(), "hello");
    EXPECT_EQ(fv.size(), 2);

    EXPECT_EQ(fv.insert(fv.begin() + 1, "test"), fv.begin() + 1);
    EXPECT_EQ(fv.at(1), "test");
    EXPECT_EQ(fv.size(), 3);

    EXPECT_EQ(fv.insert(fv.begin(), "first"), fv.begin());
    EXPECT_EQ(fv.at(0), "first");
    EXPECT_EQ(fv.size(), 4);

    EXPECT_THROW(fv.insert(fv.begin(), "overflow"), std::length_error);
    EXPECT_THROW(fv.insert(fv.end(), "overflow"), std::length_error);
}

TEST(fixed_vector, zero_capacity)
{
    using papilio::fixed_vector;

    fixed_vector<int, 0> fv;
    EXPECT_EQ(fv.capacity(), 0);

    EXPECT_THROW(fv.push_back(0), std::length_error);
}
