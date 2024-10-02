#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

TEST(variable, constructor)
{
    using namespace papilio;

    using namespace std::literals;

    static_assert(is_variable_storable_v<variable::int_type>);
    static_assert(is_variable_storable_v<variable::float_type>);
    static_assert(!is_variable_storable_v<variable::string_type>);
    static_assert(is_variable_storable_v<utf::string_container>);

    {
        variable var = true;
        EXPECT_TRUE(var.holds_bool());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = 10;
        EXPECT_TRUE(var.holds_int());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = 10u;
        EXPECT_TRUE(var.holds_int());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = 10.0f;
        EXPECT_TRUE(var.holds_float());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = 10.0;
        EXPECT_TRUE(var.holds_float());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = 10.0L;
        EXPECT_TRUE(var.holds_float());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = "test"s;
        EXPECT_TRUE(var.holds_string());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        utf::string_container sc = "test"_sc;
        sc.obtain_ownership();
        variable var = std::move(sc);
        EXPECT_TRUE(var.holds_string());
        EXPECT_TRUE(var.has_ownership());
    }

    {
        variable var = "test"_sc;
        EXPECT_TRUE(var.holds_string());
        EXPECT_FALSE(var.has_ownership());
    }

    {
        utf::string_container sc = "test"_sc;
        variable var = sc;
        EXPECT_TRUE(var.holds_string());
        EXPECT_FALSE(var.has_ownership());
    }

    {
        variable var = "test"_sr;
        EXPECT_TRUE(var.holds_string());
        EXPECT_FALSE(var.has_ownership());
    }

    {
        variable var = "test";
        EXPECT_TRUE(var.holds_string());
        EXPECT_FALSE(var.has_ownership());
    }

    {
        variable var = "test"sv;
        EXPECT_TRUE(var.holds_string());
        EXPECT_FALSE(var.has_ownership());
    }
}

TEST(variable, compare)
{
    using namespace papilio;

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

        EXPECT_FLOAT_EQ(*var.get_if<float>(), 10.0f);
        EXPECT_FLOAT_EQ(var.get<float>(), 10.0f);

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

TEST(variable, wchar_t)
{
    using namespace papilio;

    {
        wvariable var = L"test";
        EXPECT_TRUE(var.holds_string());
        EXPECT_EQ(var, L"test");
    }
}

namespace test_script_interpreter
{
template <typename... Args>
auto test_access(std::string_view fmt, Args&&... args)
{
    using namespace papilio;
    auto fmt_args = PAPILIO_NS make_format_args(std::forward<Args>(args)...);

    format_parse_context parse_ctx(fmt, fmt_args);
    parse_ctx.advance_to(parse_ctx.begin() + 1); // skip '{'

    basic_interpreter<format_context> intp;
    auto [result, it] = intp.access(parse_ctx);

    EXPECT_NE(it, parse_ctx.end());
    EXPECT_TRUE(*it == U'}' || *it == U':');

    return std::move(result);
}
} // namespace test_script_interpreter

TEST(interpreter, access)
{
    using namespace papilio;
    using namespace test_script_interpreter;

    {
        format_arg arg = test_access("{}", 182375);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        format_arg arg = test_access("{:}", 182375);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        format_arg arg = test_access("{1}", 182375, 182376);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182376);
    }

    {
        format_arg arg = test_access("{scene}", "scene"_a = 182375);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        format_arg arg = test_access("{}", "hello");

        ASSERT_TRUE(arg.holds<utf::string_container>());
        EXPECT_EQ(get<utf::string_container>(arg), "hello");
    }

    {
        auto helper = [](std::string_view fmt) -> variable
        {
            return variable(test_access(fmt, "name"_a = "Hu Tao").to_variant());
        };

        EXPECT_EQ(helper("{name.length}"), 6);

        EXPECT_EQ(helper("{name[0]}"), "H");

        EXPECT_EQ(helper("{name[-1]}"), "o");

        EXPECT_EQ(helper("{name[3:]}"), "Tao");

        EXPECT_EQ(helper("{name[-3:]}"), "Tao");

        EXPECT_EQ(helper("{name[2:3]}"), " ");

        EXPECT_EQ(helper("{name[:]}"), "Hu Tao");

        EXPECT_EQ(helper("{name[:].length}"), 6);
    }
}

TEST(interpreter, format)
{
    using namespace papilio;

    {
        interpreter intp{};

        std::string buf;
        dynamic_format_args args;
        format_context fmt_ctx(std::back_inserter(buf), args);
        format_parse_context parse_ctx("test", args);

        intp.format(parse_ctx, fmt_ctx);

        EXPECT_EQ(buf, "test");
    }
}

namespace test_script_interpreter
{
template <typename... Args>
auto get_err(papilio::format_string<Args...> fmt, Args&&... args)
{
    using namespace papilio;

    try
    {
        (void)PAPILIO_NS format(fmt, std::forward<Args>(args)...);

        throw;
    }
    catch(const script_base::error& e)
    {
        return e;
    }
}
} // namespace test_script_interpreter

TEST(interpreter, exception)
{
    using namespace papilio;
    using enum script_error_code;
    using test_script_interpreter::get_err;

    EXPECT_EQ(get_err("{").error_code(), end_of_string);
    EXPECT_EQ(get_err("{$ 'str'}").error_code(), invalid_condition);
    EXPECT_EQ(get_err("{$ 'str':}").error_code(), invalid_string);
    EXPECT_EQ(get_err("{$ 'str': 'incomplete\\").error_code(), invalid_string);
    EXPECT_EQ(get_err("{$ 'str': 'incomplete}").error_code(), end_of_string);
}

TEST(interpreter, debug)
{
    using namespace papilio;
    using enum script_error_code;

    using intp_t = basic_interpreter<format_context, true>;

    auto helper = [](utf::string_ref fmt)
    {
        dynamic_format_args args;
        format_parse_context parse_ctx(fmt, args);
        std::string str;
        format_context fmt_ctx(std::back_inserter(str), args);
        intp_t intp{};
        static_assert(intp_t::debug());

        intp.format(parse_ctx, fmt_ctx);

        FAIL() << "unreachable";
    };

#define PAPILIO_TEST_INTERPRETER_DEBUG(fmt, ec, pos)                      \
    do                                                                    \
    {                                                                     \
        utf::string_ref fmt_str = fmt;                                    \
        try                                                               \
        {                                                                 \
            return helper(fmt_str);                                       \
        }                                                                 \
        catch(const intp_t::extended_error& e)                            \
        {                                                                 \
            EXPECT_EQ(e.error_code(), ec);                                \
            EXPECT_EQ(std::distance(fmt_str.begin(), e.get_iter()), pos); \
        }                                                                 \
    } while(0)

    PAPILIO_TEST_INTERPRETER_DEBUG("{$}", invalid_condition, 2);
    PAPILIO_TEST_INTERPRETER_DEBUG("{$ 'str'}", invalid_condition, 8);
    PAPILIO_TEST_INTERPRETER_DEBUG("{$ 'str':}", invalid_string, 9);
    PAPILIO_TEST_INTERPRETER_DEBUG("{$ 'str'==={0}:'s'}", invalid_condition, 10);
}
