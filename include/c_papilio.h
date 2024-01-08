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

int papilio_format_s(papilio_context* ctx, const char* fmt, size_t fmt_sz);

int papilio_format(papilio_context* ctx, const char* fmt);

size_t papilio_get_str_size(const papilio_context* ctx);
const char* papilio_get_str(const papilio_context* ctx);

void papilio_clear_args(papilio_context* ctx);
void papilio_clear_str(papilio_context* ctx);
void papilio_reset_context(papilio_context* ctx);

#    ifdef __cplusplus
}
#    endif

#endif
