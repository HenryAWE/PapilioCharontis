#include <papilio/xchar.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
std::u8string vformat(std::u8string_view fmt, const u8format_args_ref& args)
{
    std::u8string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

std::u16string vformat(std::u16string_view fmt, const u16format_args_ref& args)
{
    std::u16string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

std::u32string vformat(std::u32string_view fmt, const u32format_args_ref& args)
{
    std::u32string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

namespace detail
{
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::u8string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char8_t>>& args
    )
    {
        using iter_t = detail::formatted_size_counter<char8_t>;
        using context_type = basic_format_context<iter_t, char8_t>;

        return vformat_to_impl<char8_t, iter_t, context_type>(
                   iter_t(),
                   loc,
                   fmt,
                   args
        )
            .get_result();
    }

    std::size_t formatted_size_impl(
        locale_ref loc,
        std::u16string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char16_t>>& args
    )
    {
        using iter_t = detail::formatted_size_counter<char16_t>;
        using context_type = basic_format_context<iter_t, char16_t>;

        return vformat_to_impl<char16_t, iter_t, context_type>(
                   iter_t(),
                   loc,
                   fmt,
                   args
        )
            .get_result();
    }

    std::size_t formatted_size_impl(
        locale_ref loc,
        std::u32string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char32_t>>& args
    )
    {
        using iter_t = detail::formatted_size_counter<char32_t>;
        using context_type = basic_format_context<iter_t, char32_t>;

        return vformat_to_impl<char32_t, iter_t, context_type>(
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
