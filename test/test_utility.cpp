#include <gtest/gtest.h>
#include <papilio/utility.hpp>


static_assert(papilio::pointer_like<std::unique_ptr<int>>);
static_assert(papilio::pointer_like<std::unique_ptr<int[]>>);
static_assert(papilio::pointer_like<std::shared_ptr<int>>);
static_assert(papilio::pointer_like<std::shared_ptr<int[]>>);
static_assert(papilio::pointer_like<int*>);
static_assert(papilio::pointer_like<int[]>);
static_assert(!papilio::pointer_like<int>);

static_assert(!papilio::string_like<unsigned char*>);
static_assert(!papilio::string_like<signed char*>);

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

TEST(independent_t, proxy)
{
    using namespace papilio;

    {
        std::string str = "hello";
        auto str_proxy = independent(str);

        static_assert(std::is_same_v<decltype(str_proxy), independent_proxy<std::string>>);

        EXPECT_EQ(&str, &str_proxy.get());
    }

    {
        std::string str = "hello";
        auto str_proxy = independent(std::as_const(str));

        static_assert(std::is_same_v<decltype(str_proxy), independent_proxy<const std::string>>);

        EXPECT_EQ(&str, &str_proxy.get());
    }

    {
        std::string str = "hello";
        auto str_proxy_1 = independent(str);
        auto str_proxy_2 = independent(str_proxy_1);
        auto str_proxy_3 = str_proxy_2;

        EXPECT_EQ(&str, &str_proxy_1.get());
        EXPECT_EQ(&str, &str_proxy_2.get());
        EXPECT_EQ(&str, &str_proxy_3.get());

        EXPECT_EQ(&str_proxy_1.get(), &str_proxy_2.get());
        EXPECT_EQ(&str_proxy_1.get(), &str_proxy_3.get());
        EXPECT_EQ(&str_proxy_2.get(), &str_proxy_3.get());
    }
}

namespace test_memory
{
    class empty_1 {};
    class empty_2 {};
}

TEST(compressed_pair, normal)
{
    using namespace papilio;

    compressed_pair<int, int> p_1{ 0, 1 };
    static_assert(sizeof(p_1) == sizeof(int) * 2);

    EXPECT_EQ(p_1.first(), 0);
    EXPECT_EQ(p_1.second(), 1);

    compressed_pair<int, int> p_2 = p_1;
    EXPECT_EQ(p_2.first(), 0);
    EXPECT_EQ(p_2.second(), 1);

    p_2.first() = 2;
    p_2.second() = 3;
    p_1.swap(p_2);
    EXPECT_EQ(p_1.first(), 2);
    EXPECT_EQ(p_1.second(), 3);
    EXPECT_EQ(p_2.first(), 0);
    EXPECT_EQ(p_2.second(), 1);
}

TEST(compressed_pair, optimized)
{
    using namespace papilio;
    using namespace test_memory;

    compressed_pair<std::string, empty_1> p_1;
    static_assert(sizeof(p_1) == sizeof(std::string));
    compressed_pair<empty_1, std::string> p_2;
    static_assert(sizeof(p_2) == sizeof(std::string));
    compressed_pair<empty_1, empty_2> p_3;
    static_assert(sizeof(p_3) == 1);
    static_assert(std::is_empty_v<compressed_pair<empty_1, empty_2>>);

    // only optimize for the first member when T1 == T2
    compressed_pair<empty_1, empty_1> p_4;
    static_assert(sizeof(p_4) <= 2);
    static_assert(!std::is_empty_v<compressed_pair<empty_1, empty_1>>);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
