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

std::string bool_to_string(bool value, const std::locale& loc)
{
    auto& f = std::use_facet<std::numpunct<char>>(loc);
    return value ?
               f.truename() :
               f.falsename();
};
} // namespace test_locale

TEST(locale, locale_ref)
{
    using namespace papilio;
    using namespace test_locale;

    // fallback to C locale
    {
        locale_ref c_loc;
        EXPECT_TRUE(std::isalpha('A', c_loc));
        EXPECT_FALSE(std::isalpha('1', c_loc));

        EXPECT_EQ(bool_to_string(true, c_loc), "true");
        EXPECT_EQ(bool_to_string(false, c_loc), "false");
    }

    // custom locale
    {
        std::locale custom(std::locale("C"), new bool_yes_no);
        locale_ref custom_ref = custom;

        EXPECT_EQ(bool_to_string(true, custom_ref), "yes");
        EXPECT_EQ(bool_to_string(false, custom_ref), "no");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
