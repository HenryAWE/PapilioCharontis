#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <papilio/iterstream.hpp>


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
    oiterstream os(std::back_inserter(buf));

    static_assert(std::is_same_v<
        decltype(os)::iterator,
        std::back_insert_iterator<std::string>
    >);

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
    woiterstream os(std::back_inserter(buf));

    static_assert(std::is_same_v<
        decltype(os)::iterator,
        std::back_insert_iterator<std::wstring>
    >);

    os << L"hello";
    os << L' ';
    os << 12345;

    EXPECT_EQ(buf, L"hello 12345");
    EXPECT_TRUE(os.good());
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
