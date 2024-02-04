#include <gtest/gtest.h>
#include <papilio/utility.hpp>
#include <map>
#include <papilio/container.hpp>

static_assert(papilio::tuple_like<std::tuple<>>);
static_assert(papilio::tuple_like<std::tuple<int>>);
static_assert(papilio::tuple_like<std::tuple<int, int, int>>);
static_assert(papilio::tuple_like<std::pair<int, int>>);
static_assert(papilio::tuple_like<std::array<int, 4>>);
static_assert(!papilio::tuple_like<int[4]>);

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

static_assert(papilio::map_like<std::map<int, int>>);
static_assert(papilio::map_like<papilio::fixed_flat_map<int, int, 8>>);

TEST(slice, slice)
{
    using namespace papilio;

    {
        constexpr slice s;
        static_assert(s.begin() == 0);
        static_assert(s.end() == slice::npos);
    }

    {
        constexpr slice s{1};
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
        slice s{1};
        EXPECT_EQ(s.begin(), 1);
        EXPECT_EQ(s.end(), slice::npos);
    }

    {
        slice s{1, 3};
        EXPECT_EQ(s.begin(), 1);
        EXPECT_EQ(s.end(), 3);
        EXPECT_EQ(s.length(), 2);
    }

    {
        slice s{1};
        slice normalized_s = s.normalize(182376);
        EXPECT_EQ(normalized_s.begin(), 1);
        EXPECT_EQ(normalized_s.end(), 182376);
        EXPECT_EQ(normalized_s.length(), 182375);
    }

    {
        slice s{-3, -1};
        EXPECT_EQ(s, (slice{-3, -1}));

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
class empty_1
{};

class empty_2
{};
} // namespace test_memory

TEST(compressed_pair, normal)
{
    using namespace papilio;

    compressed_pair<int, int> p_1{0, 1};
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

TEST(basic_iter_buf, input)
{
    using namespace papilio;

    std::string src = "12345";

    using streambuf_type = iterbuf<decltype(std::begin(src))>;

    streambuf_type sbuf(std::begin(src));
    std::istream is(&sbuf);

    char result_buf[5]{};
    is.read(result_buf, 5);
    EXPECT_EQ(std::string_view(result_buf, 5), "12345");
    EXPECT_EQ(sbuf.get(), src.end());
    EXPECT_TRUE(is.good());
}

TEST(basic_iter_buf, output)
{
    using namespace papilio;

    std::string buf;

    using streambuf_type = iterbuf<std::back_insert_iterator<std::string>>;

    streambuf_type sbuf(std::back_inserter(buf));
    std::ostream os(&sbuf);

    os << "hello";
    EXPECT_EQ(buf, "hello");
    EXPECT_TRUE(os.good());
}

TEST(basic_iter_buf, winput)
{
    using namespace papilio;

    std::wstring src = L"12345";

    using streambuf_type = witerbuf<decltype(std::begin(src))>;

    streambuf_type sbuf(std::begin(src));
    std::wistream is(&sbuf);

    wchar_t result_buf[5]{};
    is.read(result_buf, 5);
    EXPECT_EQ(std::wstring_view(result_buf, 5), L"12345");
    EXPECT_EQ(sbuf.get(), src.end());
    EXPECT_TRUE(is.good());
}

TEST(basic_iter_buf, woutput)
{
    using namespace papilio;

    std::wstring buf;

    using streambuf_type = witerbuf<std::back_insert_iterator<std::wstring>>;

    streambuf_type sbuf(std::back_inserter(buf));
    std::wostream os(&sbuf);

    os << L"hello";
    EXPECT_EQ(buf, L"hello");
    EXPECT_TRUE(os.good());
}

TEST(basic_oiterstream, char)
{
    using namespace papilio;

    std::string buf;
    oiterstream<std::back_insert_iterator<std::string>> os(
        std::back_inserter(buf)
    );

    os << "hello";
    os << ' ';
    os << 12345;

    EXPECT_EQ(buf, "hello 12345");
    EXPECT_TRUE(os.good());
}

TEST(basic_oiterstream, wchar_t)
{
    using namespace papilio;

    std::wstring buf;
    woiterstream<std::back_insert_iterator<std::wstring>> os(
        std::back_inserter(buf)
    );

    os << L"hello";
    os << L' ';
    os << 12345;

    EXPECT_EQ(buf, L"hello 12345");
    EXPECT_TRUE(os.good());
}

TEST(enum_name, enum_name)
{
    using namespace papilio;

    enum my_enum
    {
        first = 1,
        second = 2
    };

    static_assert(static_enum_name<first>(true) == "first");
    static_assert(static_enum_name<second>(true) == "second");

    EXPECT_EQ(enum_name(first, true), "first");
    EXPECT_EQ(enum_name(second, true), "second");
}

TEST(join, ostream)
{
    {
        std::stringstream ss;

        int arr[4] = {1, 2, 3, 4};
        ss << papilio::join(arr);

        EXPECT_EQ(ss.str(), "1, 2, 3, 4");
    }
    
    {
        using namespace std::literals;

        std::stringstream ss;

        int arr[4] = {1, 2, 3, 4};
        ss << papilio::join(arr, " | "sv);

        EXPECT_EQ(ss.str(), "1 | 2 | 3 | 4");
    }
    
    {
        std::stringstream ss;

        int arr[4] = {1, 2, 3, 4};
        ss << papilio::join(arr, "-");

        EXPECT_EQ(ss.str(), "1-2-3-4");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
