#ifndef PAPILIO_FORMATTER_CHRONO_HPP
#define PAPILIO_FORMATTER_CHRONO_HPP

#pragma once

#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "../format.hpp"
#include "../chrono/chrono_utility.hpp"
#include "../chrono/chrono_traits.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
namespace detail
{
    class chrono_fmt_parser_base
    {
    protected:
        static bool is_year_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
            case U'E':
                return ch == 'Y' ||
                       ch == 'y' ||
                       ch == 'C';

            case U'O':
                return ch == 'y';
            }
        }

        static bool is_month_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
                return ch == U'm' ||
                       ch == U'b' ||
                       ch == U'h' ||
                       ch == U'B';

            case U'E':
                return false;

            case U'O':
                return ch == 'm';
            }
        }

        static bool is_day_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
            case U'O':
                return ch == U'd' ||
                       ch == U'e';

            case U'E':
                return false;
            }
        }

        // Time of day, i.e. H:M:S
        static bool is_time_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
                return ch == U'H' ||
                       ch == U'I' ||
                       ch == U'M' ||
                       ch == U'S' ||
                       ch == U'R' ||
                       ch == U'T' ||
                       ch == U'r' ||
                       ch == U'p';

            case U'O':
                return ch == U'H' ||
                       ch == U'I' ||
                       ch == U'M' ||
                       ch == U'S';

            case U'E':
                return ch == U'X';
            }
        }

        // Week/day of the year
        static bool is_yday_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
                return ch == U'j' ||
                       ch == U'U' ||
                       ch == U'W';

            case U'O':
                return ch == U'U' ||
                       ch == U'W';

            case U'E':
                return false;
            }
        }

        static bool is_weekday_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
                return ch == U'a' ||
                       ch == U'A' ||
                       ch == U'u' ||
                       ch == U'w';

            case U'O':
                return ch == U'u' ||
                       ch == U'w';

            case U'E':
                return false;
            }
        }

        static bool is_date_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
                return ch == U'D' ||
                       ch == U'F' ||
                       ch == U'x';

            case U'E':
                return ch == U'x';
            }
        }

        static bool is_timezone_spec(char32_t ch, char32_t loc_ch)
        {
            switch(loc_ch)
            {
            default:
                return ch == U'z' ||
                       ch == U'Z';

            case U'O':
            case U'E':
                return ch == 'z';
            }
        }
    };
} // namespace detail

/**
 * @brief Formatter data of chrono types.
 */
PAPILIO_EXPORT template <typename CharT>
struct chrono_formatter_data
{
    using string_container_type = utf::basic_string_container<CharT>;

    simple_formatter_data basic;
    string_container_type chrono_spec;
};

/**
 * @brief Parser for chrono format specification.
 */
PAPILIO_EXPORT template <typename ParseContext>
class chrono_formatter_parser : private detail::chrono_fmt_parser_base
{
public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = chrono_formatter_data<char_type>;
    using interpreter_type = basic_interpreter<typename ParseContext::format_context_type>;

    /**
     * @brief Parse the chrono specification.
     *
     * @param ctx Parse context
     * @param comp Available components of the chrono type
     *
     * @return std::pair<result_type, iterator> Parsed format specification and current iterator position of parser
     */
    static std::pair<result_type, iterator> parse(
        ParseContext& ctx,
        chrono::components comp = chrono::components::all
    )
    {
        using chrono::components;

        result_type result;

        simple_formatter_parser<ParseContext, true> basic_parser;
        auto [basic_result, it] = basic_parser.parse(ctx);
        result.basic = basic_result;

        auto spec_start = it;
        for(; it != ctx.end(); ++it)
        {
            char32_t ch32 = *it;

            if(ch32 == '{') [[unlikely]]
                throw format_error("'{' is invalid in chrono spec");
            if(ch32 == '}')
                break;

            char32_t loc_ch = U'\0';
            if(ch32 == U'%')
            {
                ++it;
                if(it == ctx.end()) [[unlikely]]
                    throw format_error("missing format specifier after %");
                ch32 = *it;
                if(ch32 == U'E' || ch32 == U'O')
                {
                    loc_ch = ch32;
                    ++it;
                    if(it == ctx.end()) [[unlikely]]
                    {
                        throw format_error(
                            loc_ch == U'E' ? "missing format specifier after %E" : "missing format specifier after %O"
                        );
                    }
                    ch32 = *it;
                }
            }
            else
                continue;

            if(ch32 == U'%' || ch32 == U't' || ch32 == U'n')
                continue;

            if(is_year_spec(ch32, loc_ch))
            {
                if(!(comp & components::year))
                    throw format_error("no year component");
            }
            else if(is_month_spec(ch32, loc_ch))
            {
                if(!(comp & components::month))
                    throw format_error("no month component");
            }
            else if(is_day_spec(ch32, loc_ch))
            {
                if(!(comp & components::day))
                    throw format_error("no day component");
            }
            else if(is_time_spec(ch32, loc_ch))
            {
                if(!(comp & components::hour_min_sec))
                    throw format_error("no time component");
            }
            else if(is_weekday_spec(ch32, loc_ch))
            {
                if(!(comp & components::weekday))
                    throw format_error("no weekday component");
            }
            else if(loc_ch == U'\0' && (ch32 == U'q' || ch32 == U'Q')) // %q and %Q
            {
                if(!(comp & components::duration_count))
                    throw format_error("no count component");
            }
            else if(is_date_spec(ch32, loc_ch) || is_yday_spec(ch32, loc_ch))
            {
                if((comp & components::date) != components::date)
                    throw format_error("no date component");
            }
            else if(loc_ch != U'O' && ch32 == U'c') // %c and %Ec
            {
                if((comp & components::date_time) != components::date_time)
                    throw format_error("no datetime component");
            }
            else if(is_timezone_spec(ch32, loc_ch))
            {
                if(!(comp & components::time_zone))
                    throw format_error("no time zone component");
            }
            else
            {
                throw format_error("unsupported chrono spec");
            }
        }

        result.chrono_spec.assign(spec_start, it);

        return std::make_pair(std::move(result), it);
    }
};

#if defined(PAPILIO_COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wsign-conversion"
#endif

/**
 * @brief Formatter for `std::tm`
 *
 * @tparam CharT Character type
 *
 * If the specification is empty, it will format the time like `std::asctime` but without the trailing newline.
 * If the specification is not empty, it will pass the time value to `std::put_time` for converting it to string.
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<std::tm, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        chrono_formatter_parser<ParseContext> parser;
        auto [result, it] = parser.parse(ctx);

        m_data = std::move(result);

        return it;
    }

    template <typename Context>
    auto format(const std::tm& val, Context& ctx) const
        -> typename Context::iterator
    {
        if(m_data.chrono_spec.empty())
        {
            small_vector<CharT, 24> buf;
            PAPILIO_NS chrono::copy_asctime<CharT>(
                std::back_inserter(buf), val
            );

            string_formatter<CharT> fmt;
            fmt.set_data(m_data.basic);
            return fmt.format(std::basic_string_view<CharT>(buf.data(), buf.size()), ctx);
        }
        else
        {
            std::basic_stringstream<CharT> ss;
            if(m_data.basic.use_locale)
                ss.imbue(ctx.getloc());
            else
                ss.imbue(std::locale::classic());
            ss << std::put_time(&val, m_data.chrono_spec.c_str());

            string_formatter<CharT> fmt;
            fmt.set_data(m_data.basic);
            return fmt.format(std::move(ss).str(), ctx);
        }
    }

private:
    chrono_formatter_data<CharT> m_data;
};

namespace detail
{
    template <typename CharT>
    void format_century(std::basic_stringstream<CharT>& ss, int year)
    {
        PAPILIO_NS format_to(
            std::ostreambuf_iterator<CharT>(ss),
            PAPILIO_TSTRING_VIEW(CharT, "{:02}"),
            year / 100
        );
    }

    template <typename CharT>
    void format_year(std::basic_stringstream<CharT>& ss, int year, bool full)
    {
        if(full)
        {
            PAPILIO_NS format_to(
                std::ostreambuf_iterator<CharT>(ss),
                PAPILIO_TSTRING_VIEW(CharT, "{:04}"),
                year
            );
        }
        else
        {
            PAPILIO_NS format_to(
                std::ostreambuf_iterator<CharT>(ss),
                PAPILIO_TSTRING_VIEW(CharT, "{:02}"),
                year % 100
            );
        }
    }

    template <typename CharT>
    void format_weekday(std::basic_stringstream<CharT>& ss, int tm_wday, bool iso)
    {
        int day = iso ?
                      tm_wday == 0 ? 7 : tm_wday : // 1-7, 1 is Monday
                      tm_wday; // 0-6, 0 is Sunday

        PAPILIO_NS format_to(
            std::ostreambuf_iterator<CharT>(ss),
            PAPILIO_TSTRING_VIEW(CharT, "{}"),
            day
        );
    }

    template <typename CharT, typename OutputIt>
    OutputIt put_hour(OutputIt out, int hour, bool mk12)
    {
        if(mk12)
            hour = static_cast<int>(std::chrono::make12(std::chrono::hours(hour)).count());
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
            hour
        );
    }

    template <typename CharT, typename OutputIt>
    OutputIt put_am_pm(OutputIt out, bool is_am)
    {
        *out = static_cast<CharT>(is_am ? 'A' : 'P');
        ++out;
        *out = CharT('M');
        ++out;

        return out;
    }

    template <typename ChronoType>
    consteval bool has_fractional_width()
    {
        using std::is_same_v;
        using namespace std::chrono;

        if constexpr(is_specialization_of_v<ChronoType, time_point>)
            return has_fractional_width<typename ChronoType::duration>();
        if constexpr(is_specialization_of_v<ChronoType, duration>)
            return has_fractional_width<hh_mm_ss<ChronoType>>();
        else if constexpr(is_specialization_of_v<ChronoType, hh_mm_ss>)
            return ChronoType::fractional_width;
        else
            return false;
    }

    template <typename CharT, typename OutputIt>
    OutputIt put_decimal_point(OutputIt out)
    {
        *out = CharT('.');
        ++out;
        return out;
    }

    template <typename CharT, typename OutputIt, typename Rep, typename Period>
    OutputIt put_subseconds(OutputIt out, const std::chrono::duration<Rep, Period>& val)
    {
        out = put_decimal_point<CharT>(out);

        using namespace std::chrono;
        using duration_type = duration<Rep, Period>;

        auto abs_val = abs(val);
        auto frac = abs_val - duration_cast<std::chrono::seconds>(abs_val);

        using hh_mm_ss_type = hh_mm_ss<duration_type>;
        if constexpr(treat_as_floating_point_v<Rep>)
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{:0{}.0f}"),
                duration_cast<typename hh_mm_ss_type::precision>(frac).count(),
                hh_mm_ss_type::fractional_width
            );
        }
        else
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{:0{}}"),
                duration_cast<typename hh_mm_ss_type::precision>(frac).count(),
                hh_mm_ss_type::fractional_width
            );
        }
    }

    template <typename CharT, typename OutputIt, typename Clock, typename Duration>
    OutputIt put_subseconds(OutputIt out, const std::chrono::time_point<Clock, Duration>& val)
    {
        return put_subseconds<CharT>(std::move(out), val.time_since_epoch());
    }

    template <typename CharT, typename OutputIt, typename Duration>
    OutputIt put_subseconds(OutputIt out, const std::chrono::hh_mm_ss<Duration>& val)
    {
        out = put_decimal_point<CharT>(out);

        if constexpr(std::chrono::treat_as_floating_point_v<typename Duration::rep>)
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{:0{}.0f}"),
                val.subseconds().count(),
                val.fractional_width
            );
        }
        else
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{:0{}}"),
                val.subseconds().count(),
                val.fractional_width
            );
        }
    }
} // namespace detail

/**
 * @brief Base formatter of all chrono types.
 *
 * @tparam ChronoType The chrono type
 * @tparam CharT Character type
 */
PAPILIO_EXPORT template <typename ChronoType, typename CharT = char>
class chrono_formatter
{
public:
    using chrono_traits_type = chrono::chrono_traits<ChronoType>;

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        chrono_formatter_parser<ParseContext> parser;
        auto [result, it] = parser.parse(ctx, chrono_traits_type::get_components());

        m_data = result;

        return it;
    }

    template <typename Context>
    auto format(const ChronoType& val, Context& ctx) const
        -> typename Context::iterator
    {
        string_formatter<CharT> fmt;
        fmt.set_data(m_data.basic);

        if(m_data.chrono_spec.empty())
        {
            locale_ref loc = m_data.basic.use_locale ?
                                 ctx.getloc_ref() :
                                 nullptr;
            return fmt.format(
                default_impl(loc, val),
                ctx
            );
        }
        else
        {
            return fmt.format(
                to_str(ctx.getloc_ref(), val, m_data.chrono_spec, m_data.basic.use_locale),
                ctx
            );
        }
    }

private:
    chrono_formatter_data<CharT> m_data;

    static std::basic_string<CharT> default_impl(locale_ref loc, const ChronoType& val)
    {
        using std::is_same_v;
        using namespace std::chrono;

        std::basic_string<CharT> result;

        chrono_traits_type::template default_format<CharT>(
            loc, std::back_inserter(result), val
        );

        return result;
    }

    static std::basic_string<CharT> to_str(locale_ref loc, const ChronoType& val, utf::basic_string_ref<CharT> spec, bool use_locale)
    {
        PAPILIO_ASSERT(!spec.empty());

        std::tm t = chrono_traits_type::to_tm(val);
        const chrono::timezone_info& tz = PAPILIO_NS chrono::get_timezone_info(val);

        const auto sentinel = spec.end();

        std::basic_stringstream<CharT> ss;
        if(use_locale)
            ss.imbue(loc);
        const std::time_put<CharT>* facet = use_locale ?
                                                std::addressof(std::use_facet<std::time_put<CharT>>(ss.getloc())) :
                                                nullptr;

        for(auto it = spec.begin(); it != sentinel; ++it)
        {
            utf::codepoint ch = *it;
            const CharT* start = &*it;
            if(ch != U'%')
            {
                ss << ch;
                continue;
            }

            ++it;
            if(it == sentinel)
                throw_bad_format("bad chrono format spec");
            ch = *it;

            char32_t ch32 = ch;
            char32_t loc_ch = U'\0';
            if(ch32 == U'E' || ch32 == U'O')
            {
                if(it == sentinel)
                {
                    throw_bad_format("invalid format for locale");
                }

                loc_ch = ch32;
                ++it;
                ch32 = *it;
            }
            const CharT* stop = &*std::next(it);

            auto call_put_time = [=, &ss, tm_ptr = &t]()
            {
                facet->put(
                    std::ostreambuf_iterator<CharT>(ss),
                    ss,
                    CharT(' '),
                    tm_ptr,
                    start,
                    stop
                );
            };

            switch(ch32)
            {
            case U'n':
                ss << static_cast<CharT>('\n');
                continue;
            case U't':
                ss << static_cast<CharT>('\t');
                continue;
            case U'%':
                ss << static_cast<CharT>('%');
                continue;

            case U'C':
                if(use_locale && loc_ch == U'E')
                    call_put_time();
                else
                    detail::format_century(ss, t.tm_year + 1900);
                continue;

            case U'y':
                if(use_locale && loc_ch != U'\0')
                    call_put_time();
                else
                    detail::format_year(ss, t.tm_year + 1900, false);
                continue;
            case U'Y':
                if(use_locale && loc_ch == U'E')
                    call_put_time();
                else
                    detail::format_year(ss, t.tm_year + 1900, true);
                continue;

            case U'm':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                    t.tm_mon + 1
                );
                continue;

            case U'h':
            case U'b':
            case U'B':
                if(use_locale)
                    call_put_time();
                else
                {
                    PAPILIO_NS chrono::copy_month_name<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        std::chrono::month(t.tm_mon + 1),
                        ch32 == U'B'
                    );
                }
                continue;

            case U'd':
            case U'e':
                if(use_locale && loc_ch == U'O')
                    call_put_time();
                else
                {
                    PAPILIO_NS format_to(
                        std::ostreambuf_iterator<CharT>(ss),
                        ch32 == U'd' ?
                            PAPILIO_TSTRING_VIEW(CharT, "{:02d}") :
                            PAPILIO_TSTRING_VIEW(CharT, "{:2d}"),
                        t.tm_mday
                    );
                }
                continue;

                // Day of the week
            case U'u':
            case U'w':
                if(use_locale && loc_ch == U'O')
                    call_put_time();
                else
                    detail::format_weekday(ss, t.tm_wday, ch32 == U'u');
                continue;

            case U'a':
            case U'A':
                if(use_locale)
                    call_put_time();
                else
                {
                    PAPILIO_NS chrono::copy_weekday_name<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        std::chrono::weekday(t.tm_wday),
                        ch32 == U'A'
                    );
                }
                continue;

                // Day of the year
            case 'j':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:03d}"),
                    t.tm_yday + 1
                );
                continue;

            case U'H':
            case U'I':
                if(use_locale && loc_ch == U'O')
                    call_put_time();
                else
                {
                    detail::put_hour<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        t.tm_hour,
                        ch32 == U'I'
                    );
                }
                continue;

            case U'M':
                if(use_locale && loc_ch == U'O')
                    call_put_time();
                else
                {
                    PAPILIO_NS
                    format_to(
                        std::ostreambuf_iterator<CharT>(ss),
                        PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                        t.tm_min
                    );
                }
                continue;

            case U'S':
                if(use_locale && loc_ch == U'O')
                    call_put_time();
                else
                {
                    PAPILIO_NS
                    format_to(
                        std::ostreambuf_iterator<CharT>(ss),
                        PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                        t.tm_sec
                    );
                    if constexpr(PAPILIO_NS detail::has_fractional_width<ChronoType>())
                    {
                        detail::put_subseconds<CharT>(
                            std::ostreambuf_iterator<CharT>(ss),
                            val
                        );
                    }
                }
                continue;

            case 'z':
                tz.copy_offset<CharT>(std::ostreambuf_iterator<CharT>(ss), loc_ch != U'\0');
                continue;
            case 'Z':
                tz.copy_abbrev(std::ostreambuf_iterator<CharT>(ss));
                continue;

            case U'R':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}:{:02d}"),
                    t.tm_hour,
                    t.tm_min
                );
                continue;


            case U'X':
                if(use_locale)
                {
                    call_put_time();
                    continue;
                }
                [[fallthrough]];
            case U'T':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}:{:02d}:{:02d}"),
                    t.tm_hour,
                    t.tm_min,
                    t.tm_sec
                );
                if constexpr(detail::has_fractional_width<ChronoType>())
                {
                    detail::put_subseconds<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        val
                    );
                }
                continue;

            case U'r': // Equivalent to %I:%M:%S %p if use_locale is false
                if(use_locale)
                {
                    call_put_time();
                    continue;
                }

                detail::put_hour<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    t.tm_hour,
                    true
                );
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, ":{:02d}:{:02d}"),
                    t.tm_min,
                    t.tm_sec
                );
                if constexpr(detail::has_fractional_width<ChronoType>())
                {
                    detail::put_subseconds<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        val
                    );
                }
                ss.put(CharT(' '));
                detail::put_am_pm<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    std::chrono::is_am(std::chrono::hours(t.tm_hour))
                );
                continue;

            case U'p':
                if(use_locale)
                    call_put_time();
                else
                {
                    PAPILIO_NS detail::put_am_pm<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        std::chrono::is_am(std::chrono::hours(t.tm_hour))
                    );
                }
                continue;

            case U'D': // Equivalent to %m/%d/%y
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}/{:02d}/"),
                    t.tm_mon + 1,
                    t.tm_mday
                );
                detail::format_year(ss, t.tm_year + 1900, false);
                continue;
            case U'x':
                if(use_locale)
                {
                    call_put_time();
                    continue;
                }
                [[fallthrough]];
            case U'F': // Equivalent to %Y-%m-%d
                detail::format_year(ss, t.tm_year + 1900, true);
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "-{:02d}-{:02d}"),
                    t.tm_mon + 1,
                    t.tm_mday
                );
                continue;

            case U'c':
                if(use_locale)
                {
                    call_put_time();
                }
                else
                {
                    PAPILIO_NS chrono::copy_asctime(
                        std::ostreambuf_iterator<CharT>(ss), t
                    );
                }
                continue;

            case U'q':
                if constexpr(static_cast<bool>(chrono_traits_type::get_components() & chrono::components::duration_count))
                {
                    PAPILIO_NS chrono::copy_unit_suffix<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        std::in_place_type<typename ChronoType::period>
                    );
                }
                else
                    throw format_error("no count component");
                continue;
            case U'Q':
                if constexpr(static_cast<bool>(chrono_traits_type::get_components() & chrono::components::duration_count))
                {
                    PAPILIO_NS chrono::copy_count<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        val
                    );
                }
                else
                    throw format_error("no count component");
                continue;

            default:
                if(use_locale)
                {
                    call_put_time();
                }
                else
                {
                    std::string msg = "unsupported specifier %";
                    if(loc_ch != U'\0')
                        msg += static_cast<char>(loc_ch); // 'E' or 'O'
                    ch.append_to(msg);
                    throw_bad_format(msg.c_str());
                }
            }
        }

        return std::move(ss).str();
    }

    [[noreturn]]
    static void throw_bad_format(const char* msg)
    {
        throw format_error(msg);
    }
};

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

PAPILIO_EXPORT template <typename Duration, typename CharT>
class formatter<std::chrono::sys_time<Duration>, CharT> :
    public chrono_formatter<std::chrono::sys_time<Duration>, CharT>
{};

#ifndef PAPILIO_CHRONO_NO_UTC_TIME

PAPILIO_EXPORT template <typename Duration, typename CharT>
class formatter<std::chrono::utc_time<Duration>, CharT> :
    public chrono_formatter<std::chrono::utc_time<Duration>, CharT>
{};

#endif

PAPILIO_EXPORT template <typename Rep, typename Period, typename CharT>
class formatter<std::chrono::duration<Rep, Period>, CharT> :
    public chrono_formatter<std::chrono::duration<Rep, Period>, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::year_month, CharT> :
    public chrono_formatter<std::chrono::year_month, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::month_day, CharT> :
    public chrono_formatter<std::chrono::month_day, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::month_day_last, CharT> :
    public chrono_formatter<std::chrono::month_day_last, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::year_month_day, CharT> :
    public chrono_formatter<std::chrono::year_month_day, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::year_month_day_last, CharT> :
    public chrono_formatter<std::chrono::year_month_day_last, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::year, CharT> :
    public chrono_formatter<std::chrono::year, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::month, CharT> :
    public chrono_formatter<std::chrono::month, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::day, CharT> :
    public chrono_formatter<std::chrono::day, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::weekday, CharT> :
    public chrono_formatter<std::chrono::weekday, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::weekday_indexed, CharT> :
    public chrono_formatter<std::chrono::weekday_indexed, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::weekday_last, CharT> :
    public chrono_formatter<std::chrono::weekday_last, CharT>
{};

PAPILIO_EXPORT template <typename Duration, typename CharT>
class formatter<std::chrono::hh_mm_ss<Duration>, CharT> :
    public chrono_formatter<std::chrono::hh_mm_ss<Duration>, CharT>
{};

#ifndef PAPILIO_CHRONO_NO_TIMEZONE

PAPILIO_EXPORT template <typename Duration, typename TimeZonePtr, typename CharT>
class formatter<std::chrono::zoned_time<Duration, TimeZonePtr>, CharT> :
    public chrono_formatter<std::chrono::zoned_time<Duration, TimeZonePtr>, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::chrono::sys_info, CharT> :
    public chrono_formatter<std::chrono::sys_info, CharT>
{};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
