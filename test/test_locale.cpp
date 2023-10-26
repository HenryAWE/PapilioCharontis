#include <gtest/gtest.h>
#include <sstream>
#include <iomanip> // std::boolalpha
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

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
