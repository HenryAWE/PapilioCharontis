#include <papilio/format.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
std::string vformat(
    std::string_view fmt, const format_args_ref& args
)
{
    std::string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

std::string vformat(
    const std::locale& loc, std::string_view fmt, const format_args_ref& args
)
{
    std::string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), loc, fmt, args);

    return result;
}

std::wstring vformat(
    std::wstring_view fmt, const wformat_args_ref& args
)
{
    std::wstring result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

std::wstring vformat(
    const std::locale& loc, std::wstring_view fmt, const wformat_args_ref& args
)
{
    std::wstring result;
    PAPILIO_NS vformat_to(std::back_inserter(result), loc, fmt, args);

    return result;
}

namespace detail
{
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char>>& args
    )
    {
        using iter_t = detail::formatted_size_counter<char>;
        using context_type = basic_format_context<iter_t, char>;

        return vformat_to_impl<char, iter_t, context_type>(
                   iter_t(),
                   loc,
                   fmt,
                   args
        )
            .get_result();
    }

    std::size_t formatted_size_impl(
        locale_ref loc,
        std::wstring_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<wchar_t>>& args
    )
    {
        using iter_t = detail::formatted_size_counter<wchar_t>;
        using context_type = basic_format_context<iter_t, wchar_t>;

        return vformat_to_impl<wchar_t, iter_t, context_type>(
                   iter_t(),
                   loc,
                   fmt,
                   args
        )
            .get_result();
    }
} // namespace detail
} // namespace papilio

#include <papilio/detail/suffix.hpp>
