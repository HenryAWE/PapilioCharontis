#include <concepts>
#include <utility>
#include <limits>
#include <charconv>
#include <cmath>
#include "helper.hpp"

namespace papilio
{
namespace detail
{
    class std_formatter_base
    {
    private:
        std_formatter_data m_data;

    protected:
        std_formatter_data& data() noexcept
        {
            return m_data;
        }

        const std_formatter_data& data() const noexcept
        {
            return m_data;
        }

        // Returns the left and right size for filling
        std::pair<std::size_t, std::size_t> fill_size(std::size_t used) const noexcept
        {
            if(m_data.width <= used)
                return std::make_pair(0, 0);

            std::size_t remain = m_data.width - used;
            switch(m_data.align)
            {
            case format_align::right:
                return std::make_pair(remain, 0);

            case format_align::left:
                return std::make_pair(0, remain);

            case format_align::middle:
                return std::make_pair(
                    remain / 2,
                    remain / 2 + remain % 2 // ceil(remain / 2)
                );

            default:
            case format_align::default_align:
                PAPILIO_UNREACHABLE();
            }
        }

        template <typename FormatContext>
        void fill(FormatContext& ctx, std::size_t count) const
        {
            using context_t = format_context_traits<FormatContext>;

            PAPILIO_ASSERT(m_data.fill != U'\0');
            context_t::append(ctx, m_data.fill, count);
        }
    };

    inline constexpr char digit_map_lower[16] =
        {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    inline constexpr char digit_map_upper[16] =
        {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    template <std::integral T, typename CharT>
    requires(!char_like<T>)
    class int_formatter : public std_formatter_base
    {
    public:
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
        auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
        {
            CharT buf[sizeof(T) * 8];
            std::size_t buf_size = 0;

            auto [base, uppercase] = parse_type_ch(data().type);

            const auto& digits = uppercase ? digit_map_upper : digit_map_lower;

            const bool neg = val < 0;
            if constexpr(std::is_signed_v<T>)
            {
                if(neg)
                    val = -val;
            }

            do
            {
                const T t_base = static_cast<T>(base);
                const T digit = val % t_base;
                buf[buf_size++] = static_cast<CharT>(digits[digit]);
                val /= static_cast<T>(t_base);
            } while(val);

            using context_t = format_context_traits<FormatContext>;

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
    };

    template <std::floating_point T, typename CharT>
    class float_formatter : public std_formatter_base
    {
    public:
        void set_data(const std_formatter_data& dt)
        {
            PAPILIO_ASSERT(dt.contains_type(U"fFgGeEaA"));

            data() = dt;
            data().fill = dt.fill_or(U' ');
            if(data().align == format_align::default_align)
                data().align = format_align::right;
        }

        template <typename FormatContext>
        auto format(T val, FormatContext& ctx) const
        {
            using context_t = format_context_traits<FormatContext>;

            CharT buf[buf_size];

            std::size_t fp_size = 0;

            bool neg = std::signbit(val);
            val = std::abs(val);

            if(std::isinf(val)) [[unlikely]]
            {
                buf[0] = static_cast<CharT>('i');
                buf[1] = static_cast<CharT>('n');
                buf[2] = static_cast<CharT>('f');
                fp_size = 3;
            }
            else if(std::isnan(val)) [[unlikely]]
            {
                buf[0] = static_cast<CharT>('n');
                buf[1] = static_cast<CharT>('a');
                buf[2] = static_cast<CharT>('n');
                fp_size = 3;
            }
            else [[likely]]
            {
                fp_size = conv<CharT>(buf, val);
            }

            std::size_t used = fp_size;

            switch(data().sign)
            {
            case format_sign::default_sign:
            case format_sign::negative:
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

            auto [left, right] = fill_size(used);

            fill(ctx, left);

            switch(data().sign)
            {
            case format_sign::default_sign:
            case format_sign::negative:
                if(neg)
                    context_t::append(ctx, static_cast<CharT>('-'));
                break;

            case format_sign::positive:
                context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
                break;

            case format_sign::space:
                context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
                break;
            }

            context_t::append(ctx, buf, buf + fp_size);

            fill(ctx, right);

            return context_t::out(ctx);
        }

    private:
        static constexpr std::size_t buf_size = 32;

        std::pair<std::chars_format, bool> get_chars_fmt() const
        {
            std::chars_format ch_fmt{};
            bool uppercase = false;

            switch(data().type)
            {
            case U'G':
                uppercase = true;
                [[fallthrough]];
            case U'\0':
            case U'g':
                ch_fmt = std::chars_format::general;
                break;

            case 'F':
            case 'f':
                ch_fmt = std::chars_format::fixed;
                break;

            case 'A':
                uppercase = true;
                [[fallthrough]];
            case 'a':
                ch_fmt = std::chars_format::hex;
                break;

            case 'E':
                uppercase = true;
                [[fallthrough]];
            case 'e':
                ch_fmt = std::chars_format::scientific;
                break;
            }

            return std::make_pair(ch_fmt, uppercase);
        }

        template <char8_like U>
        std::size_t conv(std::span<U, buf_size> buf, T val) const
        {
            using namespace std::literals;

            std::to_chars_result result;
            auto [ch_fmt, uppercase] = get_chars_fmt();

            int precision = static_cast<int>(data().precision);
            if(precision == 0 &&
               U"fFeEgG"sv.find(data().type) != std::u32string::npos)
            {
                precision = 6;
            }

            char* start = std::bit_cast<char*>(std::to_address(buf.begin()));
            char* stop = std::bit_cast<char*>(std::to_address(buf.end()));

            if(precision == 0)
                result = std::to_chars(start, stop, val, ch_fmt);
            else
                result = std::to_chars(start, stop, val, ch_fmt, precision);

            if(result.ec == std::errc::value_too_large) [[unlikely]]
            {
                throw format_error("value too large");
            }
            else if(result.ec != std::errc())
            {
                PAPILIO_UNREACHABLE();
            }

            std::size_t size = static_cast<std::size_t>(result.ptr - buf.data());

            if(uppercase)
            {
                for(std::size_t i = 0; i < size; ++i)
                {
                    if('a' <= buf[i] && buf[i] <= 'z')
                        buf[i] -= 'a' - 'A'; // to upper
                }
            }

            return size;
        }

        template <typename U>
        requires(!char8_like<U>)
        std::size_t conv(std::span<U, buf_size> buf, T val) const
        {
            char narrow_buf[buf_size];
            std::size_t size = conv<char>(narrow_buf, val);
            PAPILIO_ASSERT(size <= buf.size());

            std::copy_n(narrow_buf, size, buf.begin());
            return size;
        }
    };

    class codepoint_formatter : public std_formatter_base
    {
    public:
        void set_data(const std_formatter_data& dt)
        {
            PAPILIO_ASSERT(dt.contains_type(U"c"));

            data() = dt;
            data().type = dt.type_or(U'c');
            data().fill = dt.fill_or(U' ');
        }

        template <typename FormatContext>
        auto format(utf::codepoint cp, FormatContext& ctx)
        {
            using context_t = format_context_traits<FormatContext>;

            auto [left, right] = fill_size(cp.estimate_width());

            fill(ctx, left);
            context_t::append(ctx, cp);
            fill(ctx, right);

            return context_t::out(ctx);
        }
    };

    template <typename CharT>
    class string_formatter : public std_formatter_base
    {
    public:
        using string_ref_type = utf::basic_string_ref<CharT>;
        using string_container_type = utf::basic_string_container<CharT>;

        void set_data(const std_formatter_data& dt)
        {
            PAPILIO_ASSERT(dt.contains_type(U"s"));

            data() = dt;
            data().type = dt.type_or(U's');
            data().fill = dt.fill_or(U' ');
            if(data().align == format_align::default_align)
                data().align = format_align::left;
        }

        template <typename FormatContext>
        auto format(string_ref_type str, FormatContext& ctx)
        {
            using context_t = format_context_traits<FormatContext>;

            std::size_t used = 0; // Used width

            // The "precision" for a string means the max width can be used.
            if(data().precision != 0)
            {
                for(auto it = str.begin(); it != str.end(); ++it)
                {
                    std::size_t w = (*it).estimate_width();
                    if(used + w > data().precision)
                    {
                        str.assign(str.begin(), it);
                        break;
                    }
                    used += w;
                }
            }
            else
            {
                for(utf::codepoint cp : str)
                    used += cp.estimate_width();
            }

            auto [left, right] = fill_size(used);

            fill(ctx, left);
            context_t::append(ctx, str);
            fill(ctx, right);

            return context_t::out(ctx);
        }
    };
} // namespace detail

PAPILIO_EXPORT template <std::integral T, typename CharT>
requires(!std::is_same_v<T, bool> && !char_like<T>)
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
    auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(m_data.type == U'c')
        {
            if(std::cmp_greater(val, std::numeric_limits<std::uint32_t>::max()))
                throw format_error("integer value out of range");

            detail::codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(val), ctx);
        }
        else
        {
            detail::int_formatter<T, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(val, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <std::floating_point T, typename CharT>
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
        std::tie(m_data, it) = parser.parse(ctx, U"fFgGeEaA"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        detail::float_formatter<T, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(val, ctx);
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<utf::codepoint, CharT>
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
    auto format(utf::codepoint cp, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U'c'))
        {
            detail::int_formatter<std::uint32_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(cp), ctx);
        }
        else
        {
            detail::codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(cp, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<bool, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"sXxBbod"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(bool val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U's'))
        {
            detail::int_formatter<std::uint8_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<std::uint8_t>(val), ctx);
        }
        else
        {
            detail::string_formatter<CharT> fmt;
            fmt.set_data(m_data);
            const auto str = get_str(val, ctx.getloc_ref());
            return fmt.format(str, ctx);
        }
    }

private:
    std_formatter_data m_data;

    utf::basic_string_container<CharT> get_str(bool val, locale_ref loc) const
    {
        if(!m_data.use_locale) [[likely]]
        {
            static constexpr CharT true_str[] = {'t', 'r', 'u', 'e'};
            std::basic_string_view<CharT> true_sv(true_str, 4);
            static constexpr CharT false_str[] = {'f', 'a', 'l', 's', 'e'};
            std::basic_string_view<CharT> false_sv(false_str, 5);

            return val ? true_sv : false_sv;
        }
        else
        {
            const auto& facet = std::use_facet<std::numpunct<CharT>>(loc);
            return val ? facet.truename() : facet.falsename();
        }
    }
};

PAPILIO_EXPORT template <typename CharT>
class formatter<utf::basic_string_container<CharT>, CharT>
{
public:
    using string_container_type = utf::basic_string_container<CharT>;

    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, true>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"s"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const string_container_type& str, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        detail::string_formatter<CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(str, ctx);
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<const void*, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, true>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"pP"sv);

        if(m_data.use_locale)
            throw format_error("invalid format");

        switch(m_data.type)
        {
        case U'\0':
        case U'p':
            m_data.type = 'x';
            break;

        case U'P':
            m_data.type = 'X';
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        m_data.alternate_form = true;

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const void* p, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        detail::int_formatter<std::uintptr_t, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(reinterpret_cast<std::uintptr_t>(p), ctx);
    }

private:
    std_formatter_data m_data{.type = U'x', .alternate_form = true};
};

namespace detail
{

    template <typename Enum, typename CharT = char>
    requires std::is_enum_v<Enum>
    class enum_formatter
    {
    public:
        template <typename ParseContext>
        auto parse(ParseContext& ctx) -> typename ParseContext::iterator
        {
            std_formatter_parser<ParseContext, true> parser;

            auto [result, it] = parser.parse(ctx, U"sBbXxod");

            m_data = result;

            return it;
        }

        template <typename FormatContext>
        auto format(Enum e, FormatContext& ctx) const -> typename FormatContext::iterator
        {
            if(m_data.type_or(U's') == U's')
            {
                detail::string_formatter<CharT> fmt;
                fmt.set_data(m_data);
                if constexpr(char8_like<CharT>)
                {
                    std::string_view name = enum_name<Enum>(e);
                    auto name_conv = std::basic_string_view<CharT>(
                        std::bit_cast<const CharT*>(name.data()),
                        name.size()
                    );
                    return fmt.format(
                        name_conv,
                        ctx
                    );
                }
                else
                {
                    utf::string_ref name = enum_name<Enum>(e);
                    auto name_conv = name.to_string_as<CharT>();
                    return fmt.format(
                        name_conv,
                        ctx
                    );
                }
            }
            else
            {
                detail::int_formatter<std::underlying_type_t<Enum>, CharT> fmt;
                fmt.set_data(m_data);
                return fmt.format(PAPILIO_NS to_underlying(e), ctx);
            }
        }

    private:
        std_formatter_data m_data{.type = U's'};
    };
} // namespace detail

template <typename T, typename CharT>
requires std::is_enum_v<T>
class formatter<T, CharT> : public detail::enum_formatter<T, CharT>
{};
} // namespace papilio
