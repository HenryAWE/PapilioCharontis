#ifndef PAPILIO_FORMATTER_FILESYSTEM_HPP
#define PAPILIO_FORMATTER_FILESYSTEM_HPP

#pragma once

#include <filesystem>
#include "../format.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
namespace detail
{
    template <typename CharT>
    utf::basic_string_container<CharT> path_to_sc(const std::filesystem::path& p, bool gen)
    {
        if(gen)
            return p.generic_string<CharT>();
        else
            return p.string<CharT>();
    }

    template <>
    utf::string_container path_to_sc<char>(const std::filesystem::path& p, bool gen);
    template <>
    utf::wstring_container path_to_sc<wchar_t>(const std::filesystem::path& p, bool gen);
} // namespace detail

template <typename CharT>
class formatter<std::filesystem::path, CharT>
{
public:
    void set_debug_format()
    {
        m_data.type = '?';
    }

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        std_formatter_parser<ParseContext, false> parser{};
        auto&& [data, it] = parser.parse(ctx, U"?g");

        m_data = data;
        return it;
    }

    template <typename Context>
    auto format(const std::filesystem::path& p, Context& ctx) const
        -> typename Context::iterator
    {
        string_formatter<CharT> fmt{};
        std_formatter_data data = m_data;
        if(m_data.type != '?')
            data.type = 's';
        fmt.set_data(data);

        return fmt.format(
            PAPILIO_NS detail::path_to_sc<CharT>(p, m_data.type == 'g'),
            ctx
        );
    }

private:
    std_formatter_data m_data;
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
