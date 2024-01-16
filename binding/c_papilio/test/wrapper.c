// The _Generic keyword is only available in C11.

#include "wrapper.h"

#define C_PAPILIO_TEST_PUSH_IMPL(type, suffix)                       \
    int test_c_papilio_push_##suffix(papilio_context* ctx, type arg) \
    {                                                                \
        return papilio_push(ctx, arg);                               \
    }

C_PAPILIO_TEST_PUSH_IMPL(int, i);
C_PAPILIO_TEST_PUSH_IMPL(long, l);
C_PAPILIO_TEST_PUSH_IMPL(long long, ll);

C_PAPILIO_TEST_PUSH_IMPL(unsigned int, ui);
C_PAPILIO_TEST_PUSH_IMPL(unsigned long, ul);
C_PAPILIO_TEST_PUSH_IMPL(unsigned long long, ull);

C_PAPILIO_TEST_PUSH_IMPL(float, f);
C_PAPILIO_TEST_PUSH_IMPL(double, lf);
C_PAPILIO_TEST_PUSH_IMPL(long double, llf);

C_PAPILIO_TEST_PUSH_IMPL(size_t, sz);
C_PAPILIO_TEST_PUSH_IMPL(intptr_t, iptr);
C_PAPILIO_TEST_PUSH_IMPL(uintptr_t, uptr);
C_PAPILIO_TEST_PUSH_IMPL(const void*, ptr);

C_PAPILIO_TEST_PUSH_IMPL(const char*, str);
