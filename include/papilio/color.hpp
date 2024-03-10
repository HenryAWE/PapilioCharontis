#ifndef PAPILIO_COLOR_HPP
#define PAPILIO_COLOR_HPP

#pragma once

#include <cstdint>
#include "format.hpp"

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat"
#endif

namespace papilio
{
PAPILIO_EXPORT class text_style;

PAPILIO_EXPORT enum class color : std::uint8_t
{
    none = 0,
    black = 30,
    red = 31,
    green = 32,
    yellow = 33,
    blue = 34,
    purple = 35,
    cyan = 36,
    white = 37
};

PAPILIO_EXPORT enum class style : std::uint8_t
{
    none = 0,
    bold = 1,
    faint = 1 << 1,
    italic = 1 << 2,
    underline = 1 << 3
};

PAPILIO_EXPORT text_style fg(color col) noexcept;

PAPILIO_EXPORT text_style bg(color col) noexcept;

PAPILIO_EXPORT class text_style
{
public:
    constexpr text_style(style st = style::none) noexcept
        : m_style(st) {}

    text_style(const text_style&) noexcept = default;

    friend text_style fg(color col) noexcept;

    friend text_style bg(color col) noexcept;

    text_style& operator|=(style rhs) noexcept
    {
        m_style = static_cast<style>(
            to_underlying(m_style) | to_underlying(rhs)
        );

        return *this;
    }

    text_style& operator|=(const text_style& rhs) noexcept
    {
        *this |= rhs.m_style;

        PAPILIO_ASSERT(!(has_foreground() && rhs.has_foreground()));
        if(!has_foreground())
            m_fg = rhs.m_fg;

        PAPILIO_ASSERT(!(has_background() && rhs.has_background()));
        if(!has_background())
            m_bg = rhs.m_bg;

        return *this;
    }

    friend text_style operator|(text_style lhs, style rhs)
    {
        lhs |= rhs;
        return lhs;
    }

    friend text_style operator|(text_style lhs, text_style rhs)
    {
        lhs |= rhs;
        return lhs;
    }

    bool has_foreground() const noexcept
    {
        return m_fg != color::none;
    }

    bool has_background() const noexcept
    {
        return m_bg != color::none;
    }

    bool has_style() const noexcept
    {
        return m_style != style::none;
    }

    bool has_style(style st) const noexcept
    {
        return to_underlying(m_style) & to_underlying(st);
    }

    template <typename Iterator>
    Iterator set(Iterator it) const
    {
        if(has_style())
        {
            if(has_style(style::bold))
                it = set_style(it, 1);
            if(has_style(style::faint))
                it = set_style(it, 2);
            if(has_style(style::italic))
                it = set_style(it, 3);
            if(has_style(style::underline))
                it = set_style(it, 4);
        }

        if(has_foreground())
        {
            it = set_color(it, to_underlying(m_fg), to_underlying(m_bg));
        }

        return it;
    }

    template <typename Iterator>
    static Iterator reset(Iterator it)
    {
        std::string_view esc = "\033[0m";
        return std::copy(esc.begin(), esc.end(), it);
    }

private:
    color m_fg = color::none;
    color m_bg = color::none;
    style m_style = style::none;

    template <typename Iterator>
    static Iterator set_style(Iterator it, std::uint8_t val)
    {
        char buf[4]{
            '\033',
            '[',
            static_cast<char>('0' + val),
            'm'
        };

        return std::copy_n(buf, 4, it);
    }

    template <typename Iterator>
    static Iterator set_color(Iterator it, std::uint8_t fg_val, std::uint8_t bg_val)
    {
        if(bg_val)
        {
            PAPILIO_ASSERT(fg_val != 0);
            bg_val += 10;

            return PAPILIO_NS format_to(it, "\033[{};{}m", fg_val, bg_val);
        }
        else
        {
            return PAPILIO_NS format_to(it, "\033[{}m", fg_val);
        }
    }
};

namespace detail
{
    template <typename T>
    class styled_arg
    {
    public:
        styled_arg(const text_style& st, const T& val)
            : m_style(st), m_ptr(std::addressof(val)) {}

        const text_style& get_style() const noexcept
        {
            return m_style;
        }

        const T& get() const noexcept
        {
            return *m_ptr;
        }

    private:
        text_style m_style;
        const T* m_ptr;
    };
} // namespace detail

PAPILIO_EXPORT template <typename T>
requires(formattable<T>)
struct formatter<detail::styled_arg<T>, char> : public formatter<T>
{
    template <typename FormatContext>
    auto format(const detail::styled_arg<T>& arg, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        const text_style& st = arg.get_style();

        bool has_style = st.has_foreground() || st.has_background() || st.has_style();
        if(has_style)
        {
            context_t::advance_to(ctx, st.set(ctx.out()));
        }

        context_t::advance_to(ctx, formatter<T>::format(arg.get(), ctx));

        if(has_style)
        {
            context_t::advance_to(ctx, st.reset(ctx.out()));
        }

        return ctx.out();
    }
};

PAPILIO_EXPORT template <typename T>
auto styled(text_style st, const T& val)
{
    return detail::styled_arg<std::remove_const_t<T>>(st, val);
}
} // namespace papilio

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

#endif
