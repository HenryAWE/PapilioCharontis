#include <gtest/gtest.h>
#include <papilio/access.hpp>


TEST(indexing_value, constructor)
{
    using namespace papilio;

    {
        indexing_value idx(0);
        ASSERT_TRUE(idx.holds_index());
        EXPECT_EQ(idx.as_index(), 0);
    }

    {
        indexing_value idx(-1);
        ASSERT_TRUE(idx.holds_index());
        EXPECT_EQ(idx.as_index(), -1);
        EXPECT_LT(idx.as_index(), 0);
    }

    {
        indexing_value idx(slice(0, 1));
        ASSERT_TRUE(idx.holds_slice());
        EXPECT_EQ(idx.as_slice().first, 0);
        EXPECT_EQ(idx.as_slice().second, 1);
    }

    {
        indexing_value idx("hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), "hello");
        EXPECT_FALSE(idx.as_string().has_ownership());
    }

    {
        indexing_value idx(independent, "hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), "hello");
        EXPECT_TRUE(idx.as_string().has_ownership());
    }
}

TEST(attribute_name, compare)
{
    using namespace std::literals;
    using papilio::attribute_name;

    auto attr = attribute_name("name");
    EXPECT_FALSE(attr.name().has_ownership());

    EXPECT_EQ("name", attr);
    EXPECT_EQ(attr, "name");
    EXPECT_EQ("name"s, attr);
    EXPECT_EQ(attr, "name"s);
    EXPECT_EQ("name"sv, attr);
    EXPECT_EQ(attr, "name"sv);
}

TEST(attribute_name, validate)
{
    using papilio::attribute_name;

    EXPECT_TRUE(attribute_name::validate("name"));
    EXPECT_TRUE(attribute_name::validate("_name"));
    EXPECT_TRUE(attribute_name::validate("NAME"));
    EXPECT_TRUE(attribute_name::validate("NAME_123"));

    EXPECT_FALSE(attribute_name::validate("123name"));
    EXPECT_FALSE(attribute_name::validate("NAME name"));
    EXPECT_FALSE(attribute_name::validate("$name"));
    EXPECT_FALSE(attribute_name::validate("!name"));
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
