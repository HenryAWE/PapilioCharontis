#include <gtest/gtest.h>
#include <papilio/container.hpp>

namespace test_container
{
// Static checks
using ffm_t = papilio::fixed_flat_map<int, int, 8, std::less<>>;
static_assert(papilio::is_transparent_v<ffm_t::value_compare>);
static_assert(std::is_empty_v<ffm_t::value_compare>);
} // namespace test_container


TEST(fixed_flat_map, emplace)
{
    using papilio::fixed_flat_map;

    fixed_flat_map<int, std::string, 4> fm;
    EXPECT_TRUE(fm.try_emplace(3, "third").second);
    EXPECT_TRUE(fm.try_emplace(1, "first").second);
    EXPECT_TRUE(fm.try_emplace(4, "fourth").second);
    EXPECT_TRUE(fm.try_emplace(2, "second").second);

    EXPECT_EQ(fm.begin()->second, "first");
    EXPECT_EQ(std::next(fm.begin())->second, "second");
    EXPECT_EQ(std::next(fm.begin(), 2)->second, "third");
    EXPECT_EQ(std::next(fm.begin(), 3)->second, "fourth");

    EXPECT_THROW(fm.try_emplace(0, "overflow"), std::length_error);
    EXPECT_THROW(fm.try_emplace(5, "overflow"), std::length_error);

    EXPECT_EQ(
        fm.try_emplace(1, "duplicated"),
        std::make_pair(fm.begin(), false)
    );

    EXPECT_EQ(fm.find(1)->second, "first");
    EXPECT_EQ(std::as_const(fm).find(1)->second, "first");
    EXPECT_EQ(fm.find(2)->second, "second");
    EXPECT_EQ(std::as_const(fm).find(2)->second, "second");
    EXPECT_EQ(fm.find(3)->second, "third");
    EXPECT_EQ(std::as_const(fm).find(3)->second, "third");
    EXPECT_EQ(fm.find(4)->second, "fourth");
    EXPECT_EQ(std::as_const(fm).find(4)->second, "fourth");
    EXPECT_EQ(fm.find(0), fm.end());
    EXPECT_EQ(std::as_const(fm).find(0), fm.end());
    EXPECT_EQ(fm.find(5), fm.end());
    EXPECT_EQ(std::as_const(fm).find(5), fm.end());

    EXPECT_EQ(fm.at(1), "first");
    EXPECT_EQ(std::as_const(fm).at(1), "first");
    EXPECT_EQ(fm.at(2), "second");
    EXPECT_EQ(std::as_const(fm).at(2), "second");
    EXPECT_THROW(fm.at(0), std::out_of_range);
    EXPECT_THROW(std::as_const(fm).at(0), std::out_of_range);
    EXPECT_THROW(fm.at(5), std::out_of_range);
    EXPECT_THROW(std::as_const(fm).at(5), std::out_of_range);

    for(int i : {1, 2, 3, 4})
        EXPECT_TRUE(fm.contains(i));
    EXPECT_FALSE(fm.contains(0));
    EXPECT_FALSE(fm.contains(5));
}

TEST(fixed_flat_map, insert_or_assign)
{
    using papilio::fixed_flat_map;

    fixed_flat_map<int, std::string, 2> fm;
    EXPECT_TRUE(fm.insert_or_assign(1, "first").second);
    EXPECT_EQ(fm.at(1), "first");
    EXPECT_FALSE(fm.insert_or_assign(1, "one").second);
    EXPECT_EQ(fm.at(1), "one");
}

TEST(fixed_flat_map, zero_capacity)
{
    using papilio::fixed_flat_map;

    fixed_flat_map<int, std::string, 0> fm;

    EXPECT_THROW(fm.try_emplace(0, "overflow"), std::length_error);
}
