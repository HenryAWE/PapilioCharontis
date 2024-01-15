#pragma once

#ifndef C_PAPILIO_H
#    define C_PAPILIO_H

#    include <stddef.h>
#    include <stdint.h>

#    include "papilio/macros.hpp"

#    ifdef __cplusplus
extern "C"
{
#    endif

typedef struct papilio_context_t papilio_context;

papilio_context* papilio_create_context(void);
void papilio_destroy_context(papilio_context* ctx);

#    define C_PAPILIO_PUSH_DECL(type, suffix) \
        int papilio_push_##suffix(papilio_context* ctx, type arg)

C_PAPILIO_PUSH_DECL(int, i);
C_PAPILIO_PUSH_DECL(long, l);
C_PAPILIO_PUSH_DECL(long long, ll);

C_PAPILIO_PUSH_DECL(unsigned int, ui);
C_PAPILIO_PUSH_DECL(unsigned long, ul);
C_PAPILIO_PUSH_DECL(unsigned long long, ull);

C_PAPILIO_PUSH_DECL(float, f);
C_PAPILIO_PUSH_DECL(double, lf);
C_PAPILIO_PUSH_DECL(long double, llf);

C_PAPILIO_PUSH_DECL(size_t, sz);
C_PAPILIO_PUSH_DECL(intptr_t, iptr);
C_PAPILIO_PUSH_DECL(uintptr_t, uptr);
C_PAPILIO_PUSH_DECL(const void*, ptr);

int papilio_push_nstr(papilio_context* ctx, const char* str, size_t sz);
int papilio_push_str(papilio_context* ctx, const char* str);

#    define papilio_push(ctx, arg) _Generic(   \
        (arg),                                 \
        int: papilio_push_i,                   \
        long: papilio_push_l,                  \
        long long: papilio_push_ll,            \
        unsigned int: papilio_push_ui,         \
        unsigned long: papilio_push_ul,        \
        unsigned long long: papilio_push_ull,  \
        float: papilio_push_f,                 \
        double: papilio_push_lf,               \
        long double: papilio_push_llf,         \
        char*: papilio_push_str,               \
        void*: papilio_push_ptr                \
    )(ctx, arg)

int papilio_vformat_s(papilio_context* ctx, const char* fmt, size_t fmt_sz);
int papilio_vformat(papilio_context* ctx, const char* fmt);

size_t papilio_get_str_size(const papilio_context* ctx);
const char* papilio_get_str(const papilio_context* ctx);

void papilio_clear_args(papilio_context* ctx);
void papilio_clear_str(papilio_context* ctx);
void papilio_clear_context(papilio_context* ctx);


#    define papilio_push_impl_1(ctx, arg) \
        papilio_push(ctx, arg)
#    define papilio_push_impl_2(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_1(ctx, __VA_ARGS__)
#    define papilio_push_impl_3(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_2(ctx, __VA_ARGS__)
#    define papilio_push_impl_4(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_3(ctx, __VA_ARGS__)
#    define papilio_push_impl_5(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_4(ctx, __VA_ARGS__)
#    define papilio_push_impl_6(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_5(ctx, __VA_ARGS__)
#    define papilio_push_impl_7(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_6(ctx, __VA_ARGS__)
#    define papilio_push_impl_8(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_7(ctx, __VA_ARGS__)
#    define papilio_push_impl_9(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_8(ctx, __VA_ARGS__)
#    define papilio_push_impl_10(ctx, arg, ...) \
        papilio_push(ctx, arg), papilio_push_impl_9(ctx, __VA_ARGS__)

#    define papilio_push_impl_select_helper( \
        _1,                                  \
        _2,                                  \
        _3,                                  \
        _4,                                  \
        _5,                                  \
        _6,                                  \
        _7,                                  \
        _8,                                  \
        _9,                                  \
        _10,                                 \
        impl,                                \
        ...                                  \
    )                                          impl

#    define papilio_push_impl_select(ctx, ...) papilio_push_impl_select_helper( \
        __VA_ARGS__,                                                            \
        papilio_push_impl_10,                                                   \
        papilio_push_impl_9,                                                    \
        papilio_push_impl_8,                                                    \
        papilio_push_impl_7,                                                    \
        papilio_push_impl_6,                                                    \
        papilio_push_impl_5,                                                    \
        papilio_push_impl_4,                                                    \
        papilio_push_impl_3,                                                    \
        papilio_push_impl_2,                                                    \
        papilio_push_impl_1                                                     \
    )(ctx, __VA_ARGS__)

#    define C_PAPILIO_MAX_FORMAT_ARGS 10u

#    ifdef PAPILIO_HAS_VA_OPT

// clang-format off

// This macro only support a limited number of format arguments, which is defined by C_PAPILIO_MAX_FORMAT_ARGS.
#        define papilio_format(ctx, fmt, ...)                              \
            (papilio_clear_context(ctx)                                    \
             __VA_OPT__(, papilio_push_impl_select(ctx, __VA_ARGS__)), \
             papilio_vformat(ctx, fmt))

// clang-format on

#    else
// PAPILIO_HAS_VA_OPT is not defined, use compatible version
// This macro has the following limitations:
// 1. Can not compile without an format argument, i.e., papilio_format(ctx, "plain text") is not supported.
//    You can use papilio_clear_context(ctx), papilio_vformat(ctx, fmt) instead.
// 2. Only support a limited number of format arguments, which is defined by C_PAPILIO_MAX_FORMAT_ARGS.
#        define papilio_format(ctx, fmt, ...)            \
            (papilio_clear_context(ctx),                 \
             papilio_push_impl_select(ctx, __VA_ARGS__), \
             papilio_vformat(ctx, fmt))
#    endif

#    ifdef __cplusplus
}
#    endif

#endif
