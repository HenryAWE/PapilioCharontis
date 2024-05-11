#include <thread>
#include <sstream>
#ifdef PAPILIO_HAS_LIB_STACKTRACE
#    include <stacktrace>
#endif
#include "../utility.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename R, typename CharT>
class formatter<joiner<R, CharT>, CharT>
{
public:
    formatter() = default;
    formatter(const formatter&) = default;

    formatter& operator=(const formatter&) = default;

    using joiner_t = joiner<R, CharT>;
    using range_type = typename joiner_t::range_type;
    using value_type = std::ranges::range_value_t<range_type>;

    template <typename ParseContext, typename FormatContext>
    requires formattable_with<value_type, FormatContext>
    auto format(const joiner_t& j, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
    {
        using formatter_t = typename FormatContext::template formatter_type<value_type>;

        if constexpr(formatter_traits<formatter_t>::template parsable<FormatContext>())
        {
            formatter_t fmt;
            parse_ctx.advance_to(fmt.parse(parse_ctx));

            bool first = true;
            for(auto&& i : j)
            {
                if(!first)
                    append_sep(fmt_ctx, j);
                first = false;

                fmt_ctx.advance_to(
                    fmt.format(i, fmt_ctx)
                );
            }
        }
        else
        {
            bool first = true;
            for(auto&& i : j)
            {
                if(!first)
                    append_sep(fmt_ctx, j);
                first = false;

                fmt_ctx.advance_to(
                    PAPILIO_NS format_to(fmt_ctx.out(), "{}", i)
                );
            }
        }

        return fmt_ctx.out();
    }

private:
    template <typename FormatContext>
    static void append_sep(FormatContext& fmt_ctx, const joiner_t& j)
    {
        using context_t = format_context_traits<FormatContext>;
        context_t::append(fmt_ctx, j.separator());
    }
};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::thread::id, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const
    {
        std::basic_stringstream<CharT> ss;
        ss << id;

        using context_t = format_context_traits<FormatContext>;
        context_t::append(ctx, std::move(ss).str());

        return ctx.out();
    }
};

#ifdef PAPILIO_HAS_LIB_STACKTRACE

namespace detail
{
    template <typename CharT, typename FormatContext>
    void stack_info_append(FormatContext& ctx, std::string_view info)
    {
        using context_t = format_context_traits<FormatContext>;

        if constexpr(char8_like<CharT>)
        {
            std::basic_string_view<CharT> sv{
                std::bit_cast<const CharT*>(info.data()),
                info.size()
            };
            context_t::append(ctx, sv);
        }
        else
        {
            context_t::append(
                ctx,
                utf::string_ref(info).to_string_as<CharT>()
            );
        }
    }
} // namespace detail

PAPILIO_EXPORT template <typename Alloc, typename CharT>
class formatter<std::basic_stacktrace<Alloc>, CharT>
{
public:
    using value_type = std::basic_stacktrace<Alloc>;

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const value_type& val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        detail::stack_info_append<CharT>(
            ctx,
            std::to_string(val)
        );

        return ctx.out();
    }
};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::stacktrace_entry, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::stacktrace_entry val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        detail::stack_info_append<CharT>(
            ctx,
            std::to_string(val)
        );

        return ctx.out();
    }
};

#endif
} // namespace papilio
