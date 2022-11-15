#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


namespace test_locale
{
    class yes_no : public std::numpunct<char>
    {
    protected:
        string_type do_truename() const override
        {
            return "yes";
        }
        string_type do_falsename() const override
        {
            return "no";
        }
    };
}

TEST(TestLocale, LocaleRef)
{
    using namespace papilio;

    {
        locale_ref c_loc;
        EXPECT_TRUE(std::isalpha('A', c_loc));
        EXPECT_FALSE(std::isalpha('1', c_loc));

        auto bool_helper = [](bool value, const std::locale& loc)->std::string
        {
            auto& f = std::use_facet<std::numpunct<char>>(loc);
            return value ?
                f.truename() :
                f.falsename();
        };

        EXPECT_EQ(bool_helper(true, c_loc), "true");
        EXPECT_EQ(bool_helper(false, c_loc), "false");

        std::locale custom(std::locale("C"), new test_locale::yes_no);
        locale_ref custom_ref = custom;

        EXPECT_EQ(bool_helper(true, custom_ref), "yes");
        EXPECT_EQ(bool_helper(false, custom_ref), "no");
    }
}
TEST(TestLocale, FormatTo)
{
    using namespace papilio;

    {
        std::vector<char> buf;

        format_to(std::back_inserter(buf), "{:L}", true);
        EXPECT_EQ(std::string_view(buf.data(), 4), "true");
        buf.clear();
        format_to(std::back_inserter(buf), "{:L}", false);
        EXPECT_EQ(std::string_view(buf.data(), 5), "false");
    }

    {
        std::vector<char> buf;
        std::locale custom(std::locale("C"), new test_locale::yes_no);

        format_to(std::back_inserter(buf), custom, "{:L}", true);
        EXPECT_EQ(std::string_view(buf.data(), 3), "yes");
        buf.clear();
        format_to(std::back_inserter(buf), custom, "{:L}", false);
        EXPECT_EQ(std::string_view(buf.data(), 2), "no");
    }
}
TEST(TestLocale, Format)
{
    using namespace papilio;

    {
        EXPECT_EQ(format("{:L}", true), "true");
        EXPECT_EQ(format("{:L}", false), "false");

        std::locale custom(std::locale("C"), new test_locale::yes_no);
        EXPECT_EQ(format(custom, "{:L}", true), "yes");
        EXPECT_EQ(format(custom, "{:L}", false), "no");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
