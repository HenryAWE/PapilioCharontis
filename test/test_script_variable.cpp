#include <gtest/gtest.h>
#include <papilio/script/variable.hpp>


TEST(variable, constructor)
{ 
    using namespace papilio;
    using namespace script;

    using namespace std::literals;

    static_assert(is_variable_storable_v<variable::int_type>);
    static_assert(is_variable_storable_v<variable::float_type>);
    static_assert(!is_variable_storable_v<variable::string_type>);
    static_assert(is_variable_storable_v<utf::string_container>);

    {
        variable var = true;
        EXPECT_TRUE(var.holds_bool());
    }

    {
        variable var = 10;
        EXPECT_TRUE(var.holds_int());
    }

    {
        variable var = 10u;
        EXPECT_TRUE(var.holds_int());
    }

    {
        variable var = 10.0f;
        EXPECT_TRUE(var.holds_float());
    }

    {
        variable var = 10.0;
        EXPECT_TRUE(var.holds_float());
    }

    {
        variable var = "test";
        EXPECT_TRUE(var.holds_string());
    }

    {
        variable var = "test"s;
        EXPECT_TRUE(var.holds_string());
    }

    {
        variable var = "test"sv;
        EXPECT_TRUE(var.holds_string());
    }
}

TEST(variable, compare)
{
    using namespace papilio;
    using script::variable;

    {
        variable var1 = 2;
        variable var2 = 3;
        EXPECT_LT(var1, var2);
    }

    {
        variable var1 = 2;
        variable var2 = 2.1f;
        EXPECT_LT(var1, var2);
    }

    {
        variable var1 = "abc";
        variable var2 = "bcd";
        EXPECT_LT(var1, var2);
    }
}

TEST(variable, equal)
{
    using namespace papilio;
    using script::variable;

    {
        variable var1 = 1;
        variable var2 = 1;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1.0f;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1.1f;
        EXPECT_TRUE(var1.equal(var2, 0.11f));
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = "abc";
        variable var2 = "abc";
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = "1";
        variable var2 = 1;
        EXPECT_NE(var1, var2);
    }

    {
        variable var1 = std::numeric_limits<float>::quiet_NaN();
        variable var2 = std::numeric_limits<float>::quiet_NaN();
        EXPECT_NE(var1, var2);
    }
}

TEST(variable, access)
{
    using namespace papilio;
    using namespace script;

    using namespace std::literals;

    {
        variable var = true;
        EXPECT_EQ(var.to_variant().index(), 0);
        EXPECT_EQ(std::as_const(var).to_variant().index(), 0);

        ASSERT_TRUE(var.get_if<bool>());
        EXPECT_TRUE(*var.get_if<bool>());
        EXPECT_TRUE(var.get<bool>());
    }

    {
        variable var = 10;
        ASSERT_TRUE(var.get_if<variable::int_type>());
        EXPECT_EQ(*var.get_if<variable::int_type>(), 10);
        EXPECT_EQ(var.get<variable::int_type>(), 10);

        EXPECT_THROW((void)var.get<bool>(), bad_variable_access);

        EXPECT_TRUE(var.as<bool>());
        EXPECT_DOUBLE_EQ(var.as<double>(), 10.0);
        EXPECT_THROW((void)var.as<utf::string_container>(), invalid_conversion);
    }

    {
        variable var = 10.0f;
        ASSERT_TRUE(var.get_if<variable::float_type>());
        EXPECT_DOUBLE_EQ(*var.get_if<variable::float_type>(), 10.0f);
        EXPECT_DOUBLE_EQ(var.get<variable::float_type>(), 10.0f);

        EXPECT_THROW((void)var.get<bool>(), bad_variable_access);

        EXPECT_TRUE(var.as<bool>());
        EXPECT_EQ(var.as<int>(), 10);
        EXPECT_THROW((void)var.as<utf::string_container>(), invalid_conversion);
    }

    {
        variable var = "test";
        EXPECT_EQ(var.get<utf::string_container>(), "test");
        EXPECT_FALSE(var.get<utf::string_container>().has_ownership());
    }

    {
        variable var = "test"s;
        EXPECT_EQ(var.get<utf::string_container>(), "test");
        EXPECT_TRUE(var.get<utf::string_container>().has_ownership());

        EXPECT_TRUE(var.as<bool>());
        EXPECT_THROW((void)var.as<variable::int_type>(), invalid_conversion);
        EXPECT_THROW((void)var.as<variable::float_type>(), invalid_conversion);
        EXPECT_EQ(var.as<std::string_view>(), "test"sv);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
