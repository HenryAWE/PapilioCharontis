#include <gtest/gtest.h>
#include <papilio/type.hpp>


static_assert(papilio::pointer_like<std::unique_ptr<int>>);
static_assert(papilio::pointer_like<std::unique_ptr<int[]>>);
static_assert(papilio::pointer_like<std::shared_ptr<int>>);
static_assert(papilio::pointer_like<std::shared_ptr<int[]>>);
static_assert(papilio::pointer_like<int*>);
static_assert(papilio::pointer_like<int[]>);
static_assert(!papilio::pointer_like<int>);

static_assert(papilio::string_like<char*>);
static_assert(papilio::string_like<const char*>);
static_assert(papilio::string_like<char[16]>);
static_assert(papilio::string_like<const char[16]>);
static_assert(papilio::string_like<char[]>);
static_assert(papilio::string_like<const char[]>);
static_assert(papilio::string_like<std::string>);
static_assert(papilio::string_like<std::string_view>);

static_assert(papilio::u8string_like<char8_t*>);
static_assert(papilio::u8string_like<const char8_t*>);
static_assert(papilio::u8string_like<char8_t[16]>);
static_assert(papilio::u8string_like<const char8_t[16]>);
static_assert(papilio::u8string_like<char8_t[]>);
static_assert(papilio::u8string_like<const char8_t[]>);
static_assert(papilio::u8string_like<std::u8string>);
static_assert(papilio::u8string_like<std::u8string_view>);

TEST(slice, slice)
{
    using namespace papilio;

    {
        constexpr slice s;
        static_assert(s.begin() == 0);
        static_assert(s.end() == slice::npos);
    }

    {
        constexpr slice s{ 1 };
        constexpr slice normalized_s = s.normalize(182376);
        static_assert(normalized_s.begin() == 1);
        static_assert(normalized_s.end() == 182376);
        static_assert(normalized_s.length() == 182375);
    }

    {
        slice s;
        EXPECT_EQ(s.begin(), 0);
        EXPECT_EQ(s.end(), slice::npos);
    }

    {
        slice s{ 1 };
        EXPECT_EQ(s.begin(), 1);
        EXPECT_EQ(s.end(), slice::npos);
    }

    {
        slice s{ 1, 3 };
        EXPECT_EQ(s.begin(), 1);
        EXPECT_EQ(s.end(), 3);
        EXPECT_EQ(s.length(), 2);
    }

    {
        slice s{ 1 };
        slice normalized_s = s.normalize(182376);
        EXPECT_EQ(normalized_s.begin(), 1);
        EXPECT_EQ(normalized_s.end(), 182376);
        EXPECT_EQ(normalized_s.length(), 182375);
    }

    {
        slice s{ -3, -1 };
        EXPECT_EQ(s, (slice{ -3, -1 }));

        slice normalized_s = s.normalize(16);
        EXPECT_EQ(normalized_s.begin(), 13);
        EXPECT_EQ(normalized_s.end(), 15);
        EXPECT_EQ(normalized_s.length(), 2);
    }
}

TEST(named_arg, named_arg)
{
    {
        using namespace papilio;

        const std::string str_val = "hello world";
        const auto a_0 = arg("string", str_val);
        EXPECT_EQ(a_0.name, "string");
        EXPECT_EQ(a_0.value, "hello world");
        EXPECT_EQ(&a_0.get(), &str_val);
        EXPECT_EQ(&static_cast<const std::string&>(a_0), &str_val);
    }

    {
        using namespace papilio::literals;

        const int int_val = 1;
        const auto a_1 = "integer"_a = int_val;
        EXPECT_EQ(a_1.name, "integer");
        EXPECT_EQ(a_1.value, int_val);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
