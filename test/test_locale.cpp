#include <gtest/gtest.h>
#include <papilio/locale.hpp>
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

namespace test_locale
{
class my_numpunct : public std::numpunct<char>
{
protected:
    string_type do_truename() const override
    {
        return "T";
    }

    string_type do_falsename() const override
    {
        return "F";
    }
};

static std::string bool_to_string(bool value, const std::locale& loc)
{
    const auto& f = std::use_facet<std::numpunct<char>>(loc);
    return value ?
               f.truename() :
               f.falsename();
};
} // namespace test_locale

TEST(Locale, LocaleRef)
{
    using namespace papilio;
    using namespace test_locale;

    // fallback to C locale
    {
        locale_ref c_loc;
        EXPECT_TRUE(c_loc.empty());

        EXPECT_TRUE(std::isalpha('A', c_loc));
        EXPECT_FALSE(std::isalpha('1', c_loc));

        EXPECT_EQ(bool_to_string(true, c_loc), "true");
        EXPECT_EQ(bool_to_string(false, c_loc), "false");
    }

    // custom locale
    {
        std::locale custom(std::locale("C"), new my_numpunct);
        locale_ref custom_ref = custom;
        EXPECT_FALSE(custom_ref.empty());

        EXPECT_EQ(bool_to_string(true, custom_ref), "T");
        EXPECT_EQ(bool_to_string(false, custom_ref), "F");
    }
}

TEST(Locale, Grouping)
{
    using namespace papilio;

    // The classic locale has no grouping: no separators may be inserted
    EXPECT_EQ(PAPILIO_NS format(std::locale::classic(), "{:L}", 1234567), "1234567");
    EXPECT_EQ(PAPILIO_NS format(std::locale::classic(), L"{:L}", 1234567), L"1234567");

    // A locale with grouping must still insert separators
    struct grouped_numpunct : public std::numpunct<char>
    {
    protected:
        string_type do_grouping() const override
        {
            return "\3";
        }

        char do_thousands_sep() const override
        {
            return ',';
        }
    };

    std::locale grouped(std::locale::classic(), new grouped_numpunct);
    EXPECT_EQ(PAPILIO_NS format(grouped, "{:L}", 1234567), "1,234,567");
}
