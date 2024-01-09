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

// clang-format off

#    define papilio_push(ctx, arg) _Generic((arg), \
        int: papilio_push_i,                       \
        long: papilio_push_l,                      \
        long long: papilio_push_ll,                \
        unsigned int: papilio_push_ui,             \
        unsigned long: papilio_push_ul,            \
        unsigned long long: papilio_push_ull,      \
        float: papilio_push_f,                     \
        double: papilio_push_lf,                   \
        long double: papilio_push_llf,             \
        char*: papilio_push_str,                   \
        default: papilio_push_ptr                  \
        )(ctx, arg)

// clang-format on

int papilio_push_nstr(papilio_context* ctx, const char* str, size_t sz);
int papilio_push_str(papilio_context* ctx, const char* str);

int papilio_vformat_s(papilio_context* ctx, const char* fmt, size_t fmt_sz);
int papilio_vformat(papilio_context* ctx, const char* fmt);

size_t papilio_get_str_size(const papilio_context* ctx);
const char* papilio_get_str(const papilio_context* ctx);

void papilio_clear_args(papilio_context* ctx);
void papilio_clear_str(papilio_context* ctx);
void papilio_clear_context(papilio_context* ctx);

#    define papilio_format_impl(ctx, fmt, arg, ...) \
        (papilio_push(ctx, arg) __VA_OPT__(, papilio_format_impl(ctx, fmt, __VA_ARGS__)))

#    define papilio_format(ctx, fmt, ...) \
        (papilio_clear_context(ctx), __VA_OPT__(papilio_format_impl(ctx, fmt, __VA_ARGS__), ) papilio_vformat(ctx, fmt))

#    ifdef __cplusplus
}
#    endif

#endif
