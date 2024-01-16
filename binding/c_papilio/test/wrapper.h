#pragma once

#ifndef C_PAPILIO_TEST_INVOKE_H
#    define C_PAPILIO_TEST_INVOKE_H

#    include <c_papilio.h>

#    ifdef __cplusplus
extern "C"
{
#    endif

#    define C_PAPILIO_TEST_PUSH_DECL(type, suffix) \
        int test_c_papilio_push_##suffix(papilio_context* ctx, type arg)

C_PAPILIO_TEST_PUSH_DECL(int, i);
C_PAPILIO_TEST_PUSH_DECL(long, l);
C_PAPILIO_TEST_PUSH_DECL(long long, ll);

C_PAPILIO_TEST_PUSH_DECL(unsigned int, ui);
C_PAPILIO_TEST_PUSH_DECL(unsigned long, ul);
C_PAPILIO_TEST_PUSH_DECL(unsigned long long, ull);

C_PAPILIO_TEST_PUSH_DECL(float, f);
C_PAPILIO_TEST_PUSH_DECL(double, lf);
C_PAPILIO_TEST_PUSH_DECL(long double, llf);

C_PAPILIO_TEST_PUSH_DECL(size_t, sz);
C_PAPILIO_TEST_PUSH_DECL(intptr_t, iptr);
C_PAPILIO_TEST_PUSH_DECL(uintptr_t, uptr);
C_PAPILIO_TEST_PUSH_DECL(const void*, ptr);

C_PAPILIO_TEST_PUSH_DECL(const char*, str);

#    ifdef __cplusplus
}
#    endif

#endif
