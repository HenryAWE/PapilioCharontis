#include <gtest/gtest.h>
#include <papilio/access.hpp>
#include <papilio/format.hpp>

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

    {
        windexing_value idx(L"hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), L"hello");
        EXPECT_FALSE(idx.as_string().has_ownership());
    }

    {
        windexing_value idx(independent, L"hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), L"hello");
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

    EXPECT_NE(attr, "{name}");
    EXPECT_NE("{name}", attr);
}

TEST(accessor, string)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{.size}", "hello"), "5");
    EXPECT_EQ(PAPILIO_NS format("{.length}", "hello"), "5");
}

TEST(accessor, tuple)
{
    using namespace papilio;

    {
        std::tuple<> empty;

        EXPECT_EQ(PAPILIO_NS format("{.size}", empty), "0");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", empty), L"0");
    }

    {
        std::pair<std::string, int> val("scene", 182376);

        EXPECT_EQ(PAPILIO_NS format("{.size}", val), "2");
        EXPECT_EQ(PAPILIO_NS format("{0.first} {0.second}", val), "scene 182376");
    }

    {
        std::pair<std::wstring, int> val(L"scene", 182376);

        EXPECT_EQ(PAPILIO_NS format(L"{.size}", val), L"2");
        EXPECT_EQ(PAPILIO_NS format(L"{0.first} {0.second}", val), L"scene 182376");
    }
}

TEST(accessor, contiguous_range)
{
    using namespace papilio;

    {
        int arr[] = {0, 1, 2, 3};
        std::span<const int> view = arr;

        EXPECT_EQ(PAPILIO_NS format("{.size}", view), "4");
        EXPECT_EQ(PAPILIO_NS format("{.size}", arr), "4");
        EXPECT_EQ(PAPILIO_NS format("{[1:4].size}", view), "3");
        EXPECT_EQ(PAPILIO_NS format("{[1:4].size}", arr), "3");
        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]},{0[2]},{0[3]}", view), "0,1,2,3");
        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]},{0[2]},{0[3]}", arr), "0,1,2,3");

        EXPECT_EQ(PAPILIO_NS format(L"{.size}", view), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", arr), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{[1:4].size}", view), L"3");
        EXPECT_EQ(PAPILIO_NS format(L"{[1:4].size}", arr), L"3");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]},{0[2]},{0[3]}", view), L"0,1,2,3");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]},{0[2]},{0[3]}", arr), L"0,1,2,3");
    }

    {
        std::array<int, 4> arr = {0, 1, 2, 3};

        EXPECT_EQ(PAPILIO_NS format("{.size}", arr), "4");
        EXPECT_EQ(PAPILIO_NS format("{.size}", arr), "4");
        EXPECT_EQ(PAPILIO_NS format("{[1:4].size}", arr), "3");
        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]},{0[2]},{0[3]}", arr), "0,1,2,3");
        
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", arr), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", arr), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{[1:4].size}", arr), L"3");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]},{0[2]},{0[3]}", arr), L"0,1,2,3");
    }

    {
        std::vector<int> vi = {0, 1};

        EXPECT_EQ(PAPILIO_NS format("{.size}", vi), "2");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", vi), L"2");

        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]}", vi), "0,1");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]}", vi), L"0,1");
    }
}

TEST(accessor, type_info)
{
    using namespace papilio;

    {
        const std::type_info& info = typeid(int);
        EXPECT_EQ(
            PAPILIO_NS format("{0.name}: {0.hash_code}", typeid(int)),
            PAPILIO_NS format("{}: {}", info.name(), info.hash_code())
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{0.name}: {0.hash_code}", typeid(int)),
            PAPILIO_NS format(L"{}: {}", utf::string_ref(info.name()).to_wstring(), info.hash_code())
        );
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
