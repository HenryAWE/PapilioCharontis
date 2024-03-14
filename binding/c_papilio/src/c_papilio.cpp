#include <papilio/papilio.hpp>
#include <cstring>
#include <cerrno>
#include <c_papilio.h>

struct papilio_context_t
{
    std::string str;
    papilio::dynamic_format_args args;
};

papilio_context* papilio_create_context(void)
{
    try
    {
        return new papilio_context();
    }
    catch(...)
    {
        errno = ENOMEM;
        return nullptr;
    }
}

void papilio_destroy_context(papilio_context* ctx)
{
    delete ctx;
}

namespace detail
{
template <typename T>
static int papilio_push_impl(papilio_context* ctx, T arg) noexcept
{
    try
    {
        ctx->args.push(std::move(arg));
        return 0;
    }
    catch(...)
    {
        errno = EINVAL;
        return -1;
    }
}
} // namespace detail

#define C_PAPILIO_PUSH_IMPL(type, suffix)                     \
    int papilio_push_##suffix(papilio_context* ctx, type arg) \
    {                                                         \
        return detail::papilio_push_impl<type>(ctx, arg);     \
    }

C_PAPILIO_PUSH_IMPL(int, i);
C_PAPILIO_PUSH_IMPL(long, l);
C_PAPILIO_PUSH_IMPL(long long, ll);

C_PAPILIO_PUSH_IMPL(unsigned int, ui);
C_PAPILIO_PUSH_IMPL(unsigned long, ul);
C_PAPILIO_PUSH_IMPL(unsigned long long, ull);

C_PAPILIO_PUSH_IMPL(float, f);
C_PAPILIO_PUSH_IMPL(double, lf);
C_PAPILIO_PUSH_IMPL(long double, llf);

C_PAPILIO_PUSH_IMPL(size_t, sz);
C_PAPILIO_PUSH_IMPL(intptr_t, iptr);
C_PAPILIO_PUSH_IMPL(uintptr_t, uptr);
C_PAPILIO_PUSH_IMPL(const void*, ptr);

int papilio_push_nstr(papilio_context* ctx, const char* str, size_t sz)
{
    std::string_view view(str, sz);
    return detail::papilio_push_impl<std::string_view>(ctx, view);
}

int papilio_push_str(papilio_context* ctx, const char* str)
{
    return papilio_push_nstr(ctx, str, std::strlen(str));
}

int papilio_vformat_s(papilio_context* ctx, const char* fmt, size_t fmt_sz)
{
    using namespace papilio;

    try
    {
        std::string_view fmt_sv(fmt, fmt_sz);
        script::interpreter intp{};
        format_parse_context parse_ctx(fmt_sv, ctx->args);
        format_context fmt_ctx(std::back_inserter(ctx->str), ctx->args);

        intp.format(parse_ctx, fmt_ctx);

        return 0;
    }
    catch(const script::script_base::error& e)
    {
        errno = EINVAL;
        return static_cast<int>(e.error_code());
    }
    catch(const papilio::format_error&)
    {
        errno = EINVAL;
        return PAPILIO_ERR_UNKNOWN_ERROR;
    }
    catch(...)
    {
        errno = EINVAL;
        return PAPILIO_ERR_UNKNOWN_ERROR;
    }
}

int papilio_vformat(papilio_context* ctx, const char* fmt)
{
    return papilio_vformat_s(ctx, fmt, strlen(fmt));
}

size_t papilio_get_str_size(const papilio_context* ctx)
{
    return ctx->str.size();
}

const char* papilio_get_str(const papilio_context* ctx)
{
    return ctx->str.c_str();
}

void papilio_clear_args(papilio_context* ctx)
{
    ctx->args.clear();
}

void papilio_clear_str(papilio_context* ctx)
{
    ctx->str.clear();
}

void papilio_clear_context(papilio_context* ctx)
{
    papilio_clear_args(ctx);
    papilio_clear_str(ctx);
}
