#include <gtest/gtest.h>
#include <cstring>
#include <c_papilio.h>
#include "wrapper.h"

TEST(c_papilio, format)
{
    papilio_context* ctx = papilio_create_context();
    ASSERT_TRUE(ctx);

#define C_PAPILIO_TEST_FORMAT_PUSH(value, result, suffix)          \
    do {                                                           \
        papilio_clear_context(ctx);                                \
        test_c_papilio_push_##suffix(ctx, value);                  \
        papilio_vformat(ctx, "{}");                                \
        EXPECT_EQ(papilio_get_str_size(ctx), std::strlen(result)); \
        EXPECT_STREQ(papilio_get_str(ctx), result);                \
    } while(0)

    C_PAPILIO_TEST_FORMAT_PUSH(182376, "182376", i);
    C_PAPILIO_TEST_FORMAT_PUSH(182376L, "182376", l);
    C_PAPILIO_TEST_FORMAT_PUSH(182376LL, "182376", ll);

    C_PAPILIO_TEST_FORMAT_PUSH(182376u, "182376", ui);
    C_PAPILIO_TEST_FORMAT_PUSH(182376uL, "182376", ul);
    C_PAPILIO_TEST_FORMAT_PUSH(182376uLL, "182376", ull);

    C_PAPILIO_TEST_FORMAT_PUSH(3.14f, "3.14", f);
    C_PAPILIO_TEST_FORMAT_PUSH(3.14, "3.14", lf);
    C_PAPILIO_TEST_FORMAT_PUSH(3.14L, "3.14", llf);

    C_PAPILIO_TEST_FORMAT_PUSH(size_t(16), "16", sz);
    C_PAPILIO_TEST_FORMAT_PUSH(intptr_t(16), "16", iptr);
    C_PAPILIO_TEST_FORMAT_PUSH(uintptr_t(16), "16", uptr);
    C_PAPILIO_TEST_FORMAT_PUSH(static_cast<void*>(NULL), "0x0", ptr);

    C_PAPILIO_TEST_FORMAT_PUSH("hello", "hello", str);

    papilio_destroy_context(ctx);
    ctx = nullptr;
}

TEST(c_papilio, error)
{
    papilio_context* ctx = papilio_create_context();
    ASSERT_TRUE(ctx);

    EXPECT_EQ(papilio_vformat(ctx, "{"), PAPILIO_ERR_END_OF_STRING);
    EXPECT_EQ(papilio_vformat(ctx, "{$ 'str'}"), PAPILIO_ERR_INVALID_CONDITION);
    EXPECT_EQ(papilio_vformat(ctx, "{$ 'str':}"), PAPILIO_ERR_INVALID_STRING);
    EXPECT_EQ(papilio_vformat(ctx, "{$ 'str': 'incomplete\\"), PAPILIO_ERR_INVALID_STRING);
    EXPECT_EQ(papilio_vformat(ctx, "{$ 'str': 'incomplete}"), PAPILIO_ERR_END_OF_STRING);

    papilio_clear_context(ctx);
    ctx = nullptr;
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
