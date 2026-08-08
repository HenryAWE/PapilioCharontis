#include <papilio/xchar.hpp>
#ifdef PAPILIO_ENABLE_XCHAR
#    include <papilio/detail/prefix.hpp>

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

// Explicit instantiations of the most common xchar paths,
// corresponding to the `extern template` declarations in the header.
namespace detail
{
    template format_iterator_for<char8_t> vformat_to_impl<
        char8_t,
        format_iterator_for<char8_t>,
        u8format_context>(
        format_iterator_for<char8_t>,
        locale_ref,
        std::u8string_view,
        const basic_format_args_ref<u8format_context>&
    );

    template format_iterator_for<char16_t> vformat_to_impl<
        char16_t,
        format_iterator_for<char16_t>,
        u16format_context>(
        format_iterator_for<char16_t>,
        locale_ref,
        std::u16string_view,
        const basic_format_args_ref<u16format_context>&
    );

    template format_iterator_for<char32_t> vformat_to_impl<
        char32_t,
        format_iterator_for<char32_t>,
        u32format_context>(
        format_iterator_for<char32_t>,
        locale_ref,
        std::u32string_view,
        const basic_format_args_ref<u32format_context>&
    );
} // namespace detail

template format_iterator_for<char8_t> vformat_to(
    format_iterator_for<char8_t>,
    std::u8string_view,
    const u8format_args_ref&
);

template format_iterator_for<char16_t> vformat_to(
    format_iterator_for<char16_t>,
    std::u16string_view,
    const u16format_args_ref&
);

template format_iterator_for<char32_t> vformat_to(
    format_iterator_for<char32_t>,
    std::u32string_view,
    const u32format_args_ref&
);

template class basic_format_arg<u8format_context>;
template class basic_format_arg<u16format_context>;
template class basic_format_arg<u32format_context>;

template class basic_interpreter<u8format_context, false>;
template class basic_interpreter<u16format_context, false>;
template class basic_interpreter<u32format_context, false>;

template class basic_interpreter_base<char8_t, false>;
template class basic_interpreter_base<char16_t, false>;
template class basic_interpreter_base<char32_t, false>;
} // namespace papilio

#endif

#include <papilio/detail/suffix.hpp>
