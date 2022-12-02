#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <iostream>
#include <papilio/detail/iterstream.hpp>


TEST(TestDetailIterStream, Iterbuf)
{
    using papilio::detail::basic_iterbuf;

    // input
    {
        std::string src = "12345";

        using streambuf_type = basic_iterbuf<
            char, decltype(std::begin(src))
        >;

        streambuf_type sbuf(std::begin(src));
        std::istream is(&sbuf);

        char result_buf[5]{};
        is.read(result_buf, 5);
        EXPECT_EQ(std::string_view(result_buf, 5), "12345");
        EXPECT_EQ(sbuf.get(), src.end());
        EXPECT_TRUE(is.good());
    }

    // output
    {
        std::string buf;

        using streambuf_type = basic_iterbuf<
            char, std::back_insert_iterator<std::string>
        >;

        streambuf_type sbuf(std::back_inserter(buf));
        std::ostream os(&sbuf);

        os << "hello";
        EXPECT_EQ(buf, "hello");
        EXPECT_TRUE(os.good());
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
