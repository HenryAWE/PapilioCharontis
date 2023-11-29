#include <gtest/gtest.h>
#include <sstream>
#include <papilio/locale.hpp>


namespace test_locale
{
    class bool_yes_no : public std::numpunct<char>
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

    std::string print_bool(bool value, const std::locale& loc)
    {
        auto& f = std::use_facet<std::numpunct<char>>(loc);
        return value ?
            f.truename() :
            f.falsename();
    };
}

TEST(locale_ref, fallback)
{
    using namespace papilio;
    using namespace test_locale;

    locale_ref c_loc;
    EXPECT_TRUE(std::isalpha('A', c_loc));
    EXPECT_FALSE(std::isalpha('1', c_loc));

    EXPECT_EQ(print_bool(true, c_loc), "true");
    EXPECT_EQ(print_bool(false, c_loc), "false");
}

TEST(locale_ref, custom_locale)
{
    using namespace papilio;
    using namespace test_locale;

    std::locale custom(std::locale("C"), new bool_yes_no);
    locale_ref custom_ref = custom;

    EXPECT_EQ(print_bool(true, custom_ref), "yes");
    EXPECT_EQ(print_bool(false, custom_ref), "no");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
