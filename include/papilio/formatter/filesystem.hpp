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

/**
 * @brief Formatter for filesystem path.
 *
 * @tparam CharT Character type
 *
 * Accepted format specification is
 * `fill-and-align width ? g`
 * All of these values are optional.
 * - fill-and-align, width: Same as the standard format specification.
 * - `?`: Writes the path as an escaped string.
 * - `g`: Writes the path in generic-format representation.
 */
template <typename CharT>
class formatter<std::filesystem::path, CharT>
{
public:
    void set_debug_format()
    {
        m_debug = true;
    }

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        simple_formatter_parser<ParseContext, false> parser{};
        auto&& [data, it] = parser.parse(ctx);
        m_data = data;

        if(it != ctx.end() && *it == U'?')
        {
            m_debug = true;
            ++it;
        }

        if(it != ctx.end() && *it == U'g')
        {
            m_gen = true;
            ++it;
        }

        return it;
    }

    template <typename Context>
    auto format(const std::filesystem::path& p, Context& ctx) const
        -> typename Context::iterator
    {
        string_formatter<CharT> fmt{};
        fmt.set_data(m_data);
        if(m_debug)
            fmt.set_debug_format();

        return fmt.format(
            PAPILIO_NS detail::path_to_sc<CharT>(p, m_gen),
            ctx
        );
    }

private:
    simple_formatter_data m_data;
    bool m_debug = false;
    bool m_gen = false;
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
