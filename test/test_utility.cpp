#include <gtest/gtest.h>
#include <papilio/utility.hpp>
#include <map>
#include <papilio/container.hpp>
#include <papilio_test/setup.hpp>

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

static_assert(papilio::is_specialization_of_v<std::vector<int>, std::vector>);
static_assert(papilio::is_specialization_of_v<std::vector<float>, std::vector>);
static_assert(!papilio::is_specialization_of_v<papilio::small_vector<float, 12>, std::vector>);

TEST(index_range, index_range)
{
    using namespace papilio;

    {
        constexpr index_range s;
        static_assert(s.begin() == 0);
        static_assert(s.end() == index_range::npos);
    }

    {
        constexpr index_range s{1};
        constexpr index_range normalized_s = s.normalize(182376);
        static_assert(normalized_s.begin() == 1);
        static_assert(normalized_s.end() == 182376);
        static_assert(normalized_s.length() == 182375);
    }

    {
        index_range s;
        EXPECT_EQ(s.begin(), 0);
        EXPECT_EQ(s.end(), index_range::npos);
    }

    {
        index_range s{1};
        EXPECT_EQ(s.begin(), 1);
        EXPECT_EQ(s.end(), index_range::npos);
    }

    {
        index_range s{1, 3};
        EXPECT_EQ(s.begin(), 1);
        EXPECT_EQ(s.end(), 3);
        EXPECT_EQ(s.length(), 2);
    }

    {
        index_range s{1};
        index_range normalized_s = s.normalize(182376);
        EXPECT_EQ(normalized_s.begin(), 1);
        EXPECT_EQ(normalized_s.end(), 182376);
        EXPECT_EQ(normalized_s.length(), 182375);
    }

    {
        index_range s{-3, -1};
        EXPECT_EQ(s, (index_range{-3, -1}));

        index_range normalized_s = s.normalize(16);
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

TEST(compressed_pair, normal)
{
    using namespace papilio;

    compressed_pair<int, int> p_1{0, 1};
    static_assert(tuple_like<decltype(p_1)>);
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

    {
        auto& [a, b] = p_1;
        EXPECT_EQ(a, 2);
        EXPECT_EQ(b, 3);
    }
}

namespace test_utility
{
static int counter_1 = 0;

class empty_1
{
public:
    empty_1()
    {
        ++counter_1;
    }
};

class empty_2
{};
} // namespace test_utility

TEST(compressed_pair, optimized)
{
    using namespace papilio;
    using namespace test_utility;

    EXPECT_EQ(counter_1, 0);

    compressed_pair<std::string, empty_1> p_1;
    static_assert(tuple_like<decltype(p_1)>);
    static_assert(sizeof(p_1) == sizeof(std::string));
    EXPECT_EQ(counter_1, 1);
    p_1.first() = "hello";
    EXPECT_EQ(std::as_const(p_1).first(), "hello");

    compressed_pair<empty_1, std::string> p_2;
    static_assert(tuple_like<decltype(p_2)>);
    static_assert(sizeof(p_2) == sizeof(std::string));
    p_2.second() = "hello";
    EXPECT_EQ(std::as_const(p_2).second(), "hello");

    compressed_pair<empty_1, empty_2> p_3;
    static_assert(tuple_like<decltype(p_3)>);
    static_assert(sizeof(p_3) == 1);
    static_assert(std::is_empty_v<compressed_pair<empty_1, empty_2>>);
    EXPECT_EQ(counter_1, 3);

    compressed_pair<empty_1, empty_1> p_4{};
    static_assert(tuple_like<decltype(p_4)>);
    static_assert(sizeof(p_4) <= 2);
    static_assert(!std::is_empty_v<compressed_pair<empty_1, empty_1>>);
    EXPECT_EQ(counter_1, 5);
}

namespace test_utility
{
template <typename CharT>
void test_iter_buf_input()
{
    std::basic_string<CharT> src = PAPILIO_TSTRING_CSTR(CharT, "12345");

    using streambuf_type = papilio::basic_iterbuf<
        CharT,
        decltype(std::begin(src))>;

    streambuf_type sbuf(std::begin(src));
    std::basic_istream<CharT> is(&sbuf);

    CharT result_buf[5]{};
    is.read(result_buf, 5);
    EXPECT_TRUE(is.good());
    EXPECT_EQ(sbuf.base(), src.end());

    const auto expected_result = PAPILIO_TSTRING_ARRAY(CharT, "12345");
    EXPECT_EQ(
        std::basic_string_view<CharT>(result_buf, 5),
        expected_result
    );
}
} // namespace test_utility

TEST(basic_iter_buf, input)
{
    using test_utility::test_iter_buf_input;

    test_iter_buf_input<char>();
    test_iter_buf_input<wchar_t>();
}

namespace test_utility
{
template <typename CharT>
void test_iter_buf_output()
{
    using string_type = std::basic_string<CharT>;

    string_type buf;

    using streambuf_type = papilio::basic_iterbuf<
        CharT,
        std::back_insert_iterator<string_type>>;
    streambuf_type sbuf(std::back_inserter(buf));
    std::basic_ostream<CharT> os(&sbuf);

    os << PAPILIO_TSTRING_CSTR(CharT, "hello");
    EXPECT_TRUE(os.good());

    const auto hello_str = PAPILIO_TSTRING_ARRAY(CharT, "hello");
    EXPECT_EQ(buf, hello_str);
}
} // namespace test_utility

TEST(basic_iter_buf, output)
{
    using test_utility::test_iter_buf_output;

    test_iter_buf_output<char>();
    test_iter_buf_output<wchar_t>();
}

namespace test_utility
{
template <typename CharT>
void test_oiterstream()
{
    using string_type = std::basic_string<CharT>;

    string_type buf;

    using os_t = papilio::basic_oiterstream<
        CharT,
        std::back_insert_iterator<string_type>>;
    os_t os(std::back_inserter(buf));

    os << PAPILIO_TSTRING_CSTR(CharT, "hello");
    os << CharT(' ');
    os << 12345;
    EXPECT_TRUE(os.good());

    const auto expected_result = PAPILIO_TSTRING_ARRAY(CharT, "hello 12345");
    EXPECT_EQ(buf, expected_result);
}
} // namespace test_utility

TEST(basic_oiterstream, output)
{
    using test_utility::test_oiterstream;

    test_oiterstream<char>();
    test_oiterstream<wchar_t>();
}

#ifdef PAPILIO_HAS_ENUM_NAME

TEST(enum_name, enum_name)
{
    using namespace papilio;

    enum my_enum
    {
        first = 1,
        second = 2
    };
    enum class my_enum_class
    {
        one = 1,
        two = 2
    };

    static_assert(static_enum_name<first>() == "first");
    static_assert(static_enum_name<second>() == "second");
    static_assert(static_enum_name<my_enum_class::one>() == "one");
    static_assert(static_enum_name<my_enum_class::two>() == "two");

    EXPECT_EQ(enum_name(first), "first");
    EXPECT_EQ(enum_name(second), "second");
    EXPECT_EQ(enum_name(my_enum_class::one), "one");
    EXPECT_EQ(enum_name(my_enum_class::two), "two");
}

#endif

TEST(tuple_for_each, tuple_for_each)
{
    using namespace papilio;

    {
        std::tuple<> empty_tp{};

        tuple_for_each(
            empty_tp,
            []() -> void
            {
                GTEST_FAIL() << "Unreachable";
            }
        );
    }

    {
        std::vector<int> result;

        std::pair<int, int> p{1, 2};
        tuple_for_each(
            p,
            [&](int v)
            { result.push_back(v); }
        );

        EXPECT_EQ(result.size(), 2);
        EXPECT_EQ(result[0], 1);
        EXPECT_EQ(result[1], 2);
    }

    {
        std::tuple<char, int, float> tp{'c', 1, 1.1f};

        std::stringstream ss;
        tuple_for_each(
            tp,
            [&](auto&& v)
            { ss << v << " "; }
        );

        EXPECT_EQ(ss.str(), "c 1 1.1 ");
    }
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
