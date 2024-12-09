#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TYPED_TEST(format_suite, script_bool_op)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

#define PAPILIO_CHECK_SCRIPT_BOOL_OP(fmt, val, expected_str) \
    do                                                       \
    {                                                        \
        string_view_type fmt_sv =                            \
            PAPILIO_TSTRING_VIEW(TypeParam, fmt);            \
        string_view_type expected_str_sv =                   \
            PAPILIO_TSTRING_VIEW(TypeParam, expected_str);   \
        EXPECT_EQ(                                           \
            PAPILIO_NS format(fmt_sv, val), expected_str_sv  \
        ) << "fmt = "                                        \
          << std::quoted(fmt)                                \
          << ", val = " << #val;                             \
    } while(0)

    PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ {}? 'true'}", 1, "true");
    PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ !{}? 'false'}", 0, "false");

    if constexpr(std::is_same_v<TypeParam, char>)
    {
        PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ {}? 'true'}", "nonempty", "true");
        PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ !{}? 'false'}", "", "false");

        PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ {val}? 'true'}", "val"_a = 1, "true");
    }
    else if constexpr(std::is_same_v<TypeParam, wchar_t>)
    {
        PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ {}? 'true'}", L"nonempty", "true");
        PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ !{}? 'false'}", L"", "false");

        PAPILIO_CHECK_SCRIPT_BOOL_OP("{$ {val}? 'true'}", L"val"_a = 1, "true");
    }
}

TYPED_TEST(format_suite, script_cmp_op)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

#define PAPILIO_CHECK_SCRIPT_CMP_OP(fmt, lhs, rhs, expected_str) \
    do                                                           \
    {                                                            \
        string_view_type fmt_sv =                                \
            PAPILIO_TSTRING_VIEW(TypeParam, fmt);                \
        string_view_type expected_str_sv =                       \
            PAPILIO_TSTRING_VIEW(TypeParam, expected_str);       \
        EXPECT_EQ(                                               \
            PAPILIO_NS format(fmt_sv, lhs, rhs), expected_str_sv \
        ) << "lhs = "                                            \
          << lhs                                                 \
          << ", rhs = "                                          \
          << rhs;                                                \
    } while(0)

#define PAPILIO_CHECK_SCRIPT_EQ_OP(lhs, rhs) \
    PAPILIO_CHECK_SCRIPT_CMP_OP("{$ {} == {}? 'eq'}", lhs, rhs, "eq")

    PAPILIO_CHECK_SCRIPT_EQ_OP(0, 0);
    PAPILIO_CHECK_SCRIPT_EQ_OP(1, 1);

#define PAPILIO_CHECK_SCRIPT_NE_OP(lhs, rhs) \
    PAPILIO_CHECK_SCRIPT_CMP_OP("{$ {} != {}? 'ne'}", lhs, rhs, "ne")

    PAPILIO_CHECK_SCRIPT_NE_OP(1, 2);
    PAPILIO_CHECK_SCRIPT_NE_OP(2, 1);

#define PAPILIO_CHECK_SCRIPT_LT_OP(lhs, rhs) \
    PAPILIO_CHECK_SCRIPT_CMP_OP("{$ {} < {}? 'lt'}", lhs, rhs, "lt")
#define PAPILIO_CHECK_SCRIPT_GT_OP(lhs, rhs) \
    PAPILIO_CHECK_SCRIPT_CMP_OP("{$ {} > {}? 'gt'}", lhs, rhs, "gt")

    PAPILIO_CHECK_SCRIPT_LT_OP(1, 2);
    PAPILIO_CHECK_SCRIPT_GT_OP(2, 1);

#define PAPILIO_CHECK_SCRIPT_LE_OP(lhs, rhs) \
    PAPILIO_CHECK_SCRIPT_CMP_OP("{$ {} <= {}? 'le'}", lhs, rhs, "le")
#define PAPILIO_CHECK_SCRIPT_GE_OP(lhs, rhs) \
    PAPILIO_CHECK_SCRIPT_CMP_OP("{$ {} >= {}? 'ge'}", lhs, rhs, "ge")

    PAPILIO_CHECK_SCRIPT_LE_OP(1, 2);
    PAPILIO_CHECK_SCRIPT_GE_OP(2, 1);
    PAPILIO_CHECK_SCRIPT_LE_OP(1, 1);
    PAPILIO_CHECK_SCRIPT_GE_OP(1, 1);
}

TYPED_TEST(format_suite, script_branch)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

    const TypeParam expected_a[] = {'a', '\0'};
    const TypeParam expected_b[] = {'b', '\0'};
    const TypeParam expected_c[] = {'c', '\0'};

    {
        string_view_type script =
            PAPILIO_TSTRING_VIEW(TypeParam, "{$ {}? 'a' : ${}? 'b' : 'c'}");

        EXPECT_EQ(PAPILIO_NS format(script, true, true), expected_a);
        EXPECT_EQ(PAPILIO_NS format(script, true, false), expected_a);
        EXPECT_THROW((void)PAPILIO_NS format(script, true), std::out_of_range);

        EXPECT_EQ(PAPILIO_NS format(script, false, true), expected_b);
        EXPECT_EQ(PAPILIO_NS format(script, false, false), expected_c);
    }

    {
        string_view_type script =
            PAPILIO_TSTRING_VIEW(TypeParam, "{$ {}? 'a' : ${}? 'b' : ${}? 'c'}");

        EXPECT_EQ(PAPILIO_NS format(script, true, true, false), expected_a);
        EXPECT_EQ(PAPILIO_NS format(script, true, false, false), expected_a);

        EXPECT_EQ(PAPILIO_NS format(script, false, true, true), expected_b);
        EXPECT_EQ(PAPILIO_NS format(script, false, false, true), expected_c);
        EXPECT_EQ(PAPILIO_NS format(script, false, false, false), string_view_type());
    }
}

TYPED_TEST(format_suite, script_composite)
{
    using namespace papilio;

    using string_view_type = typename TestFixture::string_view_type;

#define PAPILIO_CHECK_FORMAT_EQ(expected_str, fmt, ...)    \
    do                                                     \
    {                                                      \
        string_view_type fmt_sv =                          \
            PAPILIO_TSTRING_VIEW(TypeParam, fmt);          \
        string_view_type expected_str_sv =                 \
            PAPILIO_TSTRING_VIEW(TypeParam, expected_str); \
        EXPECT_EQ(                                         \
            PAPILIO_NS format(fmt_sv, __VA_ARGS__),        \
            expected_str_sv                                \
        );                                                 \
    } while(0)

    PAPILIO_CHECK_FORMAT_EQ(
        "182375 182376", "{} {}", 182375, 182376
    );

    {
        const auto hello_str = PAPILIO_TSTRING_ARRAY(TypeParam, "hello");

        PAPILIO_CHECK_FORMAT_EQ(
            "***5", "{.length:*>4}", hello_str
        );
        PAPILIO_CHECK_FORMAT_EQ(
            "length is 5", "length is {.length}", hello_str
        );
    }

    {
        const string_view_type expected_results[]{
            PAPILIO_TSTRING_VIEW(TypeParam, "0 warnings"),
            PAPILIO_TSTRING_VIEW(TypeParam, "1 warning"),
            PAPILIO_TSTRING_VIEW(TypeParam, "2 warnings"),
        };

        string_view_type fmt = PAPILIO_TSTRING_VIEW(TypeParam, "{0} warning{${0}!=1?'s'}");

        for(std::size_t i : {0, 1, 2})
        {
            EXPECT_EQ(
                PAPILIO_NS format(fmt, i),
                expected_results[i]
            ) << "i = "
              << i;
        }
    }

    {
        const string_view_type expected_results[]{
            PAPILIO_TSTRING_VIEW(TypeParam, "There are 0 apples"),
            PAPILIO_TSTRING_VIEW(TypeParam, "There is 1 apple"),
            PAPILIO_TSTRING_VIEW(TypeParam, "There are 2 apples"),
        };

        string_view_type fmt = PAPILIO_TSTRING_VIEW(
            TypeParam,
            "There"
            " {${0} != 1? 'are' : 'is'} "
            "{0}"
            " apple{${0} != 1? 's'}"
        );

        for(int i : {0, 1, 2})
        {
            EXPECT_EQ(
                PAPILIO_NS format(fmt, i),
                expected_results[i]
            ) << "i = "
              << i;
        }
    }

    {
        const string_view_type expected_results[]{
            PAPILIO_TSTRING_VIEW(TypeParam, "zero"),
            PAPILIO_TSTRING_VIEW(TypeParam, "1"),
            PAPILIO_TSTRING_VIEW(TypeParam, "2"),
        };

        string_view_type fmt = PAPILIO_TSTRING_VIEW(TypeParam, "{${0}==0? 'zero' : {0}}");

        for(int i : {0, 1, 2})
        {
            EXPECT_EQ(
                PAPILIO_NS format(fmt, i),
                expected_results[i]
            ) << "i = "
              << i;
        }
    }
}
