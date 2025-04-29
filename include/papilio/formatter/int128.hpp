#ifndef PAPILIO_FORMATTER_INT128_HPP
#define PAPILIO_FORMATTER_INT128_HPP

#pragma once

#include "../detail/config.hpp"
#include "../format.hpp"

#ifdef PAPILIO_STDLIB_MSVC_STL
#    define PAPILIO_IMPL_INT128_MSVC_STL
#    define PAPILIO_HAS_INT128 "std::_Unsigned128/_Signed128"
// 128-bit integer provided by MSVC STL
#    include <__msvc_int128.hpp>
#endif

#ifndef PAPILIO_HAS_INT128
#    ifdef __SIZEOF_INT128__
// Built-in __int128 provided by GCC/Clang extension
#        define PAPILIO_IMPL_INT128_EXT__INT128
#        define PAPILIO_HAS_INT128 "(unsigned) __int128"
#    endif
#endif

#include "../detail/prefix.hpp"

namespace papilio
{
#ifdef PAPILIO_IMPL_INT128_MSVC_STL

using int128_t = std::_Signed128;
using uint128_t = std::_Unsigned128;

namespace detail
{
    template <typename Int128>
    struct int128_is_unsigned;

    template <>
    struct int128_is_unsigned<std::_Signed128> : std::false_type
    {};

    template <>
    struct int128_is_unsigned<std::_Unsigned128> : std::true_type
    {};

    // Check if val < 0
    constexpr bool i128_signbit(const std::_Signed128& val) noexcept
    {
        return val._Word[1] >> 63uLL;
    }

    constexpr std::_Signed128 i128_abs(const std::_Signed128& val) noexcept
    {
        if(i128_signbit(val))
            return -val;
        else
            return val;
    }

    constexpr std::uint64_t i128_low64bit(const std::_Signed128& val) noexcept
    {
        return val._Word[0];
    }

    constexpr std::uint64_t u128_low64bit(const std::_Unsigned128& val) noexcept
    {
        return val._Word[0];
    }
} // namespace detail

#endif

#ifdef PAPILIO_IMPL_INT128_EXT__INT128

#    ifdef PAPILIO_COMPILER_GCC
#        pragma GCC diagnostic push
// Ignoring "ISO C++ does not support '__int128' for 'type name'"
#        pragma GCC diagnostic ignored "-Wpedantic"
#    endif

using int128_t = __int128;
using uint128_t = unsigned __int128;

namespace detail
{
    template <typename Int128>
    struct int128_is_unsigned;

    template <>
    struct int128_is_unsigned<unsigned __int128> : std::false_type
    {};

    template <>
    struct int128_is_unsigned<__int128> : std::true_type
    {};

    // Check if val < 0
    constexpr bool i128_signbit(const __int128& val) noexcept
    {
        return static_cast<bool>(val >> 127);
    }

    constexpr __int128 i128_abs(const __int128& val) noexcept
    {
        if(PAPILIO_NS detail::i128_signbit(val))
            return -val;
        else
            return val;
    }

    constexpr std::uint64_t i128_low64bit(const __int128& val) noexcept
    {
        return static_cast<std::uint64_t>(val & (~0uLL));
    }

    constexpr std::uint64_t u128_low64bit(const unsigned __int128& val) noexcept
    {
        return static_cast<std::uint64_t>(val & (~0uLL));
    }
} // namespace detail

#    ifdef PAPILIO_COMPILER_GCC
#        pragma GCC diagnostic pop
#    endif

#endif

#ifdef PAPILIO_HAS_INT128

template <typename T>
concept integral_128bit =
    std::same_as<std::remove_cv_t<T>, int128_t> ||
    std::same_as<std::remove_cv_t<T>, uint128_t>;

template <typename T, typename CharT>
class int128_formatter : public std_formatter_base
{
public:
    using char_type = char;
    using facet_type = std::numpunct<CharT>;

    constexpr void set_data(const std_formatter_data& dt) noexcept
    {
        PAPILIO_ASSERT(dt.contains_type(U"BbXxod"));

        data() = dt;
        data().fill = dt.fill_or(U' ');
        data().type = dt.type_or(U'd');
        if(data().align != format_align::default_align)
            data().fill_zero = false;
        else
            data().align = format_align::right;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        if constexpr(context_t::use_locale())
        {
            if(data().use_locale)
            {
                return format_by_facet(val, ctx);
            }
        }

        CharT buf[16 * 8];
        std::size_t buf_size = 0;

        auto [base, uppercase] = parse_type_ch(data().type);

        const auto& digits = uppercase ? digit_map_upper : digit_map_lower;

        const bool neg = val < 0;

        if constexpr(!detail::int128_is_unsigned<T>::value)
        {
            do
            {
                const T digit = detail::i128_abs(val % static_cast<std::uint64_t>(base));
                buf[buf_size++] = static_cast<CharT>(digits[detail::i128_low64bit(digit)]);
                val /= base;
            } while(val);
        }
        else
        {
            do
            {
                const T digit = val % static_cast<T>(base);
                buf[buf_size++] = static_cast<CharT>(digits[detail::u128_low64bit(digit)]);
                val /= static_cast<T>(base);
            } while(val);
        }

        std::size_t used = buf_size;
        if(data().alternate_form)
            used += alt_prefix_width(base);
        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                ++used;
            break;

        case format_sign::positive:
        case format_sign::space:
            ++used;
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        auto [left, right] = data().fill_zero ?
                                 std::make_pair<std::size_t, std::size_t>(0, 0) :
                                 fill_size(used);

        fill(ctx, left);

        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                context_t::append(ctx, static_cast<CharT>('-'));
            break;

        case format_sign::positive:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
            break;

        case format_sign::space:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        if(data().alternate_form && base != 10)
        {
            context_t::append(ctx, '0');
            switch(base)
            {
            case 16:
                context_t::append(ctx, uppercase ? 'X' : 'x');
                break;

            case 2:
                context_t::append(ctx, uppercase ? 'B' : 'b');
                break;

            default:
                break;
            }
        }

        if(data().fill_zero)
        {
            if(used < data().width)
            {
                context_t::append(
                    ctx,
                    static_cast<CharT>('0'),
                    data().width - used
                );
            }
        }

        for(std::size_t i = buf_size; i > 0; --i)
            context_t::append(ctx, buf[i - 1]);

        fill(ctx, right);

        return context_t::out(ctx);
    }

private:
    // Returns the number base and whether to use uppercase.
    static std::pair<int, bool> parse_type_ch(char32_t ch) noexcept
    {
        int base = 10;
        bool uppercase = false;

        switch(ch)
        {
        case U'X':
            uppercase = true;
            [[fallthrough]];
        case U'x':
            base = 16;
            break;

        case U'B':
            uppercase = true;
            [[fallthrough]];
        case U'b':
            base = 2;
            break;

        case U'O':
            uppercase = true;
            [[fallthrough]];
        case U'o':
            base = 8;
            break;

        case U'D':
            uppercase = true;
            [[fallthrough]];
        case U'd':
            PAPILIO_ASSERT(base == 10);
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        return std::make_pair(base, uppercase);
    }

    // Get width of the prefix of alternate form
    static std::size_t alt_prefix_width(int base) noexcept
    {
        switch(base)
        {
        case 10:
            return 0;

        case 2:
        case 16:
            return 2; // "0b", "0B", "0x" and "0X"

        case 8:
            return 1; // "o"

        default:
            PAPILIO_UNREACHABLE();
        }
    }

    template <typename Context>
    auto format_by_facet(T val, Context& ctx) const
    {
        using context_t = format_context_traits<Context>;

        small_vector<CharT, 256> buf;
        const facet_type& facet = std::use_facet<facet_type>(context_t::getloc_ref(ctx));

        auto [base, uppercase] = parse_type_ch(data().type);

        const auto& digits = uppercase ? digit_map_upper : digit_map_lower;

        const bool neg = val < 0;

        std::size_t used = 0;
        std::size_t digit_count = 0;

        std::string grouping = facet.grouping();
        CharT sep = facet.thousands_sep();
        const std::size_t sep_width = utf::codepoint(static_cast<char32_t>(sep)).estimate_width();

        auto write_buf = [&, sep_idx = std::size_t(0), count_since_sep = std::size_t(0)](CharT ch) mutable
        {
            if(digit_count != 0)
            {
                char current_grouping_val = PAPILIO_NS index_grouping(grouping, sep_idx);
                if(count_since_sep >= std::size_t(current_grouping_val))
                {
                    buf.push_back(sep);
                    used += sep_width;
                    count_since_sep = 0;
                    ++sep_idx;
                }
            }

            buf.push_back(ch);
            ++digit_count;
            ++count_since_sep;
            ++used;
        };

        if constexpr(!detail::int128_is_unsigned<T>::value)
        {
            do
            {
                const T digit = detail::i128_abs(val % static_cast<T>(base));
                write_buf(static_cast<CharT>(digits[detail::i128_low64bit(digit)]));
                val /= static_cast<T>(base);
            } while(val);
        }
        else
        {
            do
            {
                const T digit = val % static_cast<T>(base);
                write_buf(static_cast<CharT>(digits[detail::u128_low64bit(digit)]));
                val /= static_cast<T>(base);
            } while(val);
        }

        if(data().alternate_form)
            used += alt_prefix_width(base);
        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                ++used;
            break;

        case format_sign::positive:
        case format_sign::space:
            ++used;
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        auto [left, right] = data().fill_zero ?
                                 std::make_pair<std::size_t, std::size_t>(0, 0) :
                                 fill_size(used);

        fill(ctx, left);

        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                context_t::append(ctx, static_cast<CharT>('-'));
            break;

        case format_sign::positive:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
            break;

        case format_sign::space:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        if(data().alternate_form && base != 10)
        {
            context_t::append(ctx, '0');
            switch(base)
            {
            case 16:
                context_t::append(ctx, uppercase ? 'X' : 'x');
                break;

            case 2:
                context_t::append(ctx, uppercase ? 'B' : 'b');
                break;

            default:
                break;
            }
        }

        if(data().fill_zero)
        {
            if(digit_count < data().width)
            {
                std::size_t zeros = data().width - digit_count;
                for(std::size_t i = 0; i < zeros; ++i)
                    write_buf(CharT('0'));
            }
        }

        for(std::size_t i = buf.size(); i > 0; --i)
            context_t::append(ctx, buf[i - 1]);

        fill(ctx, right);

        return context_t::out(ctx);
    }
};

template <integral_128bit T, typename CharT>
class formatter<T, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"XxBbodc"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const T& val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        if(m_data.type == U'c')
        {
            if(std::cmp_greater(static_cast<std::uint64_t>(val), std::numeric_limits<std::uint32_t>::max()))
                throw format_error("integer value out of range");

            codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(static_cast<std::uint64_t>(val)), ctx);
        }
        else
        {
            int128_formatter<T, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(val, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
