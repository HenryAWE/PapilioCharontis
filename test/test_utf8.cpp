#include <gtest/gtest.h>
#include <papilio/utf8.hpp>
#include <papilio/core.hpp> // slice


TEST(TestUTF8, Codepoint)
{
    using namespace papilio;

    {
        auto [cp, len] = utf8::codepoint::decode(u8"A");
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(len, 1);

        std::tie(cp, len) = utf8::codepoint::decode(u8"ü");
        EXPECT_EQ(cp, U'ü');
        EXPECT_EQ(len, 2);

        std::tie(cp, len) = utf8::codepoint::decode(u8"我");
        EXPECT_EQ(cp, U'我');
        EXPECT_EQ(len, 3);

        std::tie(cp, len) = utf8::codepoint::decode(u8"🔊");
        EXPECT_EQ(cp, U'🔊');
        EXPECT_EQ(len, 4);

        std::u8string_view str = u8"我们";
        std::tie(cp, len) = utf8::codepoint::decode(str);
        EXPECT_EQ(cp, U'我');
        EXPECT_EQ(len, 3);

        str = str.substr(len);
        std::tie(cp, len) = utf8::codepoint::decode(str);
        EXPECT_EQ(cp, U'们');
        EXPECT_EQ(len, 3);
    }

    {
        auto [cp, len] = utf8::codepoint::rdecode(u8"A");
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(len, 1);

        std::tie(cp, len) = utf8::codepoint::rdecode(u8"ü");
        EXPECT_EQ(cp, U'ü');
        EXPECT_EQ(len, 2);

        std::tie(cp, len) = utf8::codepoint::rdecode(u8"我");
        EXPECT_EQ(cp, U'我');
        EXPECT_EQ(len, 3);
        
        std::tie(cp, len) = utf8::codepoint::rdecode(u8"🔊");
        EXPECT_EQ(cp, U'🔊');
        EXPECT_EQ(len, 4);

        std::u8string_view str = u8"我们";
        std::tie(cp, len) = utf8::codepoint::rdecode(str);
        EXPECT_EQ(cp, U'们');
        EXPECT_EQ(len, 3);

        str = str.substr(0, str.size() - len);
        std::tie(cp, len) = utf8::codepoint::rdecode(str);
        EXPECT_EQ(cp, U'我');
        EXPECT_EQ(len, 3);
    }

    {
        utf8::codepoint cp(u8"a", 1);
        EXPECT_EQ(cp.size(), 1);
        EXPECT_EQ(cp.to_int().first, U'a');

        cp.assign(u8"ü", 2);
        EXPECT_EQ(cp.size(), 2);
        EXPECT_EQ(cp.to_int().first, U'ü');

        cp.assign(u8"我", 3);
        EXPECT_EQ(cp.size(), 3);
        EXPECT_EQ(cp.to_int().first, U'我');

        cp.assign(u8"🔊", 4);
        EXPECT_EQ(cp.size(), 4);
        EXPECT_EQ(cp.to_int().first, U'🔊');
    }

    {
        utf8::codepoint cp = U'A';
        std::u8string_view view = cp;
        EXPECT_EQ(view, u8"A");

        cp = U'ü';
        view = cp;
        EXPECT_EQ(view, u8"ü");

        cp = U'我';
        view = cp;
        EXPECT_EQ(view, u8"我");;

        cp = U'们';
        view = cp;
        EXPECT_EQ(view, u8"们");
    }

    {
        using namespace literals;
        auto cp = u8"我"_cp;
        EXPECT_EQ(cp.to_int().first, U'我');

        cp = u8"们"_cp;
        EXPECT_EQ(cp, u8"们"_cp);
    }

    {
        using namespace literals;

        EXPECT_EQ(u8"A"_cp, U'A');
        EXPECT_EQ(u8"ü"_cp, U'ü');
        EXPECT_EQ(u8"我"_cp, U'我');

        EXPECT_LT(u8"A"_cp, U'B');
        EXPECT_GT(u8"B"_cp, U'A');
    }
}
TEST(TestUTF8, Utilities)
{
    using namespace papilio;

    // length
    {
        EXPECT_EQ(utf8::strlen((const char*)u8"你好，世界"), 5);
    }
    
    // substring
    {
        std::string src = (const char*)u8"你好，世界！";
        EXPECT_EQ(utf8::substr(src, 1), (const char*)u8"好，世界！");
        EXPECT_EQ(utf8::substr(src, 3, 2), (const char*)u8"世界");
        EXPECT_EQ(utf8::substr(src, 0, 1), (const char*)u8"你");
    }

    // indexing
    {
        std::string src = (const char*)u8"你好，世界！";
        EXPECT_EQ(utf8::index(src, 0), (const char*)u8"你");
        EXPECT_EQ(utf8::index(src, 1), (const char*)u8"好");
        EXPECT_EQ(utf8::index(src, 2), (const char*)u8"，");
        EXPECT_EQ(utf8::index(src, 3), (const char*)u8"世");
        EXPECT_EQ(utf8::index(src, 4), (const char*)u8"界");
        EXPECT_EQ(utf8::index(src, 5), (const char*)u8"！");
        EXPECT_EQ(utf8::index(src, 6), std::string());
    }

    // reversely indexing
    {
        std::string src = (const char*)u8"你好，世界！";
        EXPECT_EQ(utf8::rindex(src, 0), (const char*)u8"！");
        EXPECT_EQ(utf8::rindex(src, 1), (const char*)u8"界");
        EXPECT_EQ(utf8::rindex(src, 2), (const char*)u8"世");
        EXPECT_EQ(utf8::rindex(src, 6), std::string());
    }

    // slicing
    {
        std::string str = "hello world!";

        EXPECT_EQ(utf8::substr(str, slice(0, 5)), "hello");
        EXPECT_EQ(utf8::substr(str, slice(6, 13)), "world!");
        EXPECT_EQ(utf8::substr(str, slice(6, slice::npos)), "world!");
        EXPECT_EQ(utf8::substr(str, slice(-6, 13)), "world!");
        EXPECT_EQ(utf8::substr(str, slice(-6, -1)), "world");
        EXPECT_EQ(utf8::substr(str, slice(-slice::npos, -1)), "hello world");
    }

    // slicing for non-ASCII characters
    {
        std::string str = (const char*)u8"你好，世界！";

        EXPECT_EQ(utf8::substr(str, slice(0, 2)), (const char*)u8"你好");
        EXPECT_EQ(utf8::substr(str, slice(-slice::npos, -1)), (const char*)u8"你好，世界");
        EXPECT_EQ(utf8::substr(str, slice(-3, slice::npos)), (const char*)u8"世界！");
    }
}
TEST(TestUTF8, StringContainer)
{
    using namespace papilio;

    {
        string_container str = "borrowed";
        EXPECT_TRUE(str.is_borrowed());
        EXPECT_EQ(str, "borrowed");
        EXPECT_EQ("borrowed", str);

        str.make_independent();
        EXPECT_FALSE(str.is_borrowed());
    }

    {
        string_container str = independent("hello world");
        EXPECT_EQ(str.length(), str.size());

        EXPECT_EQ(str[0], U'h');
        EXPECT_EQ(str[-1], U'd');

        EXPECT_FALSE(str.is_borrowed());
        auto borrowed_hello = str.substr(0, 5);
        EXPECT_TRUE(borrowed_hello.is_borrowed());
        EXPECT_EQ(borrowed_hello, "hello");
        auto hello = str.substr(independent, 0, 5);
        EXPECT_FALSE(hello.is_borrowed());
        EXPECT_EQ(hello, borrowed_hello);

        auto borrowed_world = str.substr(slice(-5, slice::npos));
        EXPECT_TRUE(borrowed_world.is_borrowed());
        EXPECT_EQ(borrowed_world, "world");
    }

    {
        string_container non_ascii = (const char*)u8"非ASCII字符串";
        EXPECT_NE(non_ascii.length(), non_ascii.size());
        EXPECT_EQ(non_ascii.length(), 9);

        EXPECT_EQ(non_ascii[0], U'非');
        EXPECT_EQ(non_ascii[1], U'A');
        EXPECT_EQ(non_ascii.substr(1, 5), "ASCII");
        EXPECT_EQ(non_ascii.substr(slice(-3, slice::npos)), (const char*)u8"字符串");
        EXPECT_EQ(non_ascii.substr(9), std::string_view());
    }

    {
        string_container str = (const char*)u8"你";
        EXPECT_TRUE(str.is_borrowed());
        str.push_back(U'好');
        EXPECT_FALSE(str.is_borrowed());
        EXPECT_EQ(str, (const char*)u8"你好");
        EXPECT_EQ(str.length(), 2);
        EXPECT_EQ(str.front(), U'你');
        EXPECT_EQ(str.back(), U'好');

        str.pop_back();
        EXPECT_FALSE(str.is_borrowed());
        EXPECT_EQ(str.length(), 1);
        EXPECT_EQ(str.front(), str.back());
        EXPECT_EQ(str.front(), U'你');

        str.clear();
        EXPECT_TRUE(str.is_borrowed());
        EXPECT_TRUE(str.empty());
    }

    {
        string_container str = "123";
        EXPECT_TRUE(str.is_borrowed());

        str.pop_back();
        EXPECT_TRUE(str.is_borrowed());
        EXPECT_EQ(str, "12");
    }

    {
        string_container str = "hello world";
        std::u32string result;
        for(auto&& cp : str)
            result += cp;
        EXPECT_EQ(result, U"hello world");
    }

    {
        string_container str = "hello world";
        std::u32string result;
        for(auto&& cp : str)
            result += cp;
        EXPECT_EQ(result, U"hello world");
        EXPECT_TRUE(str.contains(U'h'));
        EXPECT_TRUE(str.contains("hello"));
        EXPECT_TRUE(str.contains("world"));

        EXPECT_NE(str.find(U'h'), str.cend());
        EXPECT_NE(str.find("hello"), str.cend());
        EXPECT_NE(str.find("world"), str.cend());
        EXPECT_EQ(str.find(U'h', 1), str.cend());
        EXPECT_EQ(str.find("hello", 1), str.cend());
        EXPECT_NE(str.find("world", 1), str.cend());
    }

    {
        string_container str = (const char*)u8"你好，世界！";
        // GoogleTest cannot correctly handle char8_t
        EXPECT_TRUE(str.to_u8string() == u8"你好，世界！");
        std::u32string result;
        for(auto&& cp : str)
            result += cp;
        EXPECT_EQ(result, U"你好，世界！");
        EXPECT_EQ(result, str.to_u32string());
        EXPECT_EQ(str.to_u32string(), U"你好，世界！");
        EXPECT_TRUE(str.contains(U'你'));
        EXPECT_TRUE(str.contains((const char*)u8"你好"));
        EXPECT_TRUE(str.contains((const char*)u8"世界"));

        EXPECT_NE(str.find(U'你'), str.cend());
        EXPECT_NE(str.find((const char*)u8"你好"), str.cend());
        EXPECT_NE(str.find((const char*)u8"世界"), str.cend());
        EXPECT_EQ(str.find(U'你', 1), str.cend());
        EXPECT_EQ(str.find((const char*)u8"你好", 1), str.cend());
        EXPECT_NE(str.find((const char*)u8"世界", 1), str.cend());

        std::reverse(result.begin(), result.end());
        EXPECT_TRUE(std::equal(result.begin(), result.end(), str.rbegin(), str.rend()));
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
