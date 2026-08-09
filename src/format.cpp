#include <papilio/format.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
std::string vformat(
    std::string_view fmt, const format_args_ref& args
)
{
    std::string result;
    PAPILIO_NS detail::vformat_to_impl<char, format_iterator_for<char>, format_context>(
        std::back_inserter(result),
        nullptr,
        fmt,
        args
    );

    return result;
}

std::string vformat(
    const std::locale& loc, std::string_view fmt, const format_args_ref& args
)
{
    std::string result;
    PAPILIO_NS detail::vformat_to_impl<char, format_iterator_for<char>, format_context>(
        std::back_inserter(result),
        loc,
        fmt,
        args
    );

    return result;
}

std::wstring vformat(
    std::wstring_view fmt, const wformat_args_ref& args
)
{
    std::wstring result;
    PAPILIO_NS detail::vformat_to_impl<wchar_t, format_iterator_for<wchar_t>, wformat_context>(
        std::back_inserter(result),
        nullptr,
        fmt,
        args
    );

    return result;
}

std::wstring vformat(
    const std::locale& loc, std::wstring_view fmt, const wformat_args_ref& args
)
{
    std::wstring result;
    PAPILIO_NS detail::vformat_to_impl<wchar_t, format_iterator_for<wchar_t>, wformat_context>(
        std::back_inserter(result),
        loc,
        fmt,
        args
    );

    return result;
}

namespace detail
{
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::string_view fmt,
        const format_args_ref& args
    )
    {
        std::basic_string<char> buf;
        vformat_to_impl<char, format_iterator_for<char>, format_context>(
            std::back_inserter(buf),
            loc,
            fmt,
            args
        );
        return buf.size();
    }

    std::size_t formatted_size_impl(
        locale_ref loc,
        std::wstring_view fmt,
        const wformat_args_ref& args
    )
    {
        std::basic_string<wchar_t> buf;
        vformat_to_impl<wchar_t, format_iterator_for<wchar_t>, wformat_context>(
            std::back_inserter(buf),
            loc,
            fmt,
            args
        );
        return buf.size();
    }
} // namespace detail

// Explicit instantiations of the most common paths,
// corresponding to the `extern template` declarations in the headers.
namespace detail
{
    template format_iterator_for<char> vformat_to_impl<
        char,
        format_iterator_for<char>,
        format_context>(
        format_iterator_for<char>,
        locale_ref,
        std::string_view,
        const basic_format_args_ref<format_context>&
    );

    template format_iterator_for<wchar_t> vformat_to_impl<
        wchar_t,
        format_iterator_for<wchar_t>,
        wformat_context>(
        format_iterator_for<wchar_t>,
        locale_ref,
        std::wstring_view,
        const basic_format_args_ref<wformat_context>&
    );
} // namespace detail

template format_iterator_for<char> vformat_to(
    format_iterator_for<char>,
    std::string_view,
    const format_args_ref&
);

template format_iterator_for<char> vformat_to(
    format_iterator_for<char>,
    const std::locale&,
    std::string_view,
    const format_args_ref&
);

template format_iterator_for<wchar_t> vformat_to(
    format_iterator_for<wchar_t>,
    std::wstring_view,
    const wformat_args_ref&
);

template format_iterator_for<wchar_t> vformat_to(
    format_iterator_for<wchar_t>,
    const std::locale&,
    std::wstring_view,
    const wformat_args_ref&
);

template class basic_format_arg<format_context>;
template class basic_format_arg<wformat_context>;

template class basic_interpreter<format_context, false>;
template class basic_interpreter<wformat_context, false>;

template class basic_interpreter_base<char, false>;
template class basic_interpreter_base<wchar_t, false>;
} // namespace papilio

#include <papilio/detail/suffix.hpp>
