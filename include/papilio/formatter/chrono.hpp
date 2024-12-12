#ifndef PAPILIO_FORMATTER_CHRONO_HPP
#define PAPILIO_FORMATTER_CHRONO_HPP

#pragma once

#include <ctime>
#include <span>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "../format.hpp"
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

PAPILIO_EXPORT template <typename CharT>
struct chrono_formatter_data
{
    using string_container_type = utf::basic_string_container<CharT>;

    simple_formatter_data basic;
    string_container_type chrono_spec;
};

template <typename ParseContext>
PAPILIO_EXPORT class chrono_formatter_parser : private detail::chrono_fmt_parser_base
{
public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = chrono_formatter_data<char_type>;
    using interpreter_type = basic_interpreter<typename ParseContext::format_context_type>;

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

template <typename CharT>
constexpr CharT weekday_names_short[7][3] = {
    {'S', 'u', 'n'},
    {'M', 'o', 'n'},
    {'T', 'u', 'e'},
    {'W', 'e', 'd'},
    {'T', 'h', 'u'},
    {'F', 'r', 'i'},
    {'S', 'a', 't'}
};

template <typename CharT>
inline constexpr CharT month_names_short[12][3] = {
    {'J', 'a', 'n'},
    {'F', 'e', 'b'},
    {'M', 'a', 'r'},
    {'A', 'p', 'r'},
    {'M', 'a', 'y'},
    {'J', 'u', 'n'},
    {'J', 'u', 'l'},
    {'A', 'u', 'g'},
    {'S', 'e', 'p'},
    {'O', 'c', 't'},
    {'N', 'o', 'v'},
    {'D', 'e', 'c'}
};

template <typename CharT>
inline constexpr std::basic_string_view<CharT> weekday_names_full[7] = {
    PAPILIO_TSTRING_VIEW(CharT, "Sunday"),
    PAPILIO_TSTRING_VIEW(CharT, "Monday"),
    PAPILIO_TSTRING_VIEW(CharT, "Tuesday"),
    PAPILIO_TSTRING_VIEW(CharT, "Wednesday"),
    PAPILIO_TSTRING_VIEW(CharT, "Thursday"),
    PAPILIO_TSTRING_VIEW(CharT, "Friday"),
    PAPILIO_TSTRING_VIEW(CharT, "Saturday"),
};

template <typename CharT>
inline constexpr std::basic_string_view<CharT> month_names_full[12] = {
    PAPILIO_TSTRING_VIEW(CharT, "January"),
    PAPILIO_TSTRING_VIEW(CharT, "February"),
    PAPILIO_TSTRING_VIEW(CharT, "March"),
    PAPILIO_TSTRING_VIEW(CharT, "April"),
    PAPILIO_TSTRING_VIEW(CharT, "May"),
    PAPILIO_TSTRING_VIEW(CharT, "June"),
    PAPILIO_TSTRING_VIEW(CharT, "July"),
    PAPILIO_TSTRING_VIEW(CharT, "August"),
    PAPILIO_TSTRING_VIEW(CharT, "September"),
    PAPILIO_TSTRING_VIEW(CharT, "October"),
    PAPILIO_TSTRING_VIEW(CharT, "November"),
    PAPILIO_TSTRING_VIEW(CharT, "December"),
};

#if defined(PAPILIO_COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wsign-conversion"
#endif

namespace detail
{
    /**
    * @brief Similar to `std::asctime()` but without the trailing newline
    *
    * @param out Output iterator
    * @param t Time value
    */
    template <typename CharT = char, typename OutputIt>
    OutputIt put_asctime(OutputIt out, const std::tm& t)
    {
        int wday = std::clamp(t.tm_wday, 0, 6);
        int mon = std::clamp(t.tm_mon, 0, 11);

        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{} {} {:2d} {:02d}:{:02d}:{:02d} {:4d}"),
            std::basic_string_view<CharT>(weekday_names_short<CharT>[wday], 3),
            std::basic_string_view<CharT>(month_names_short<CharT>[mon], 3),
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec,
            t.tm_year + 1900
        );
    }
} // namespace detail

template <typename CharT>
class formatter<std::tm, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        chrono_formatter_parser<ParseContext> parser;
        auto [result, it] = parser.parse(ctx);

        m_data = result;

        return it;
    }

    template <typename Context>
    auto format(const std::tm& val, Context& ctx) const
        -> typename Context::iterator
    {
        if(m_data.chrono_spec.empty())
        {
            small_vector<CharT, 24> buf;
            PAPILIO_NS detail::put_asctime<CharT>(
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
    OutputIt put_weekday_by_name(OutputIt out, const std::chrono::weekday& wd, bool fullname)
    {
        if(!wd.ok()) [[unlikely]]
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "weekday({})"),
                wd.c_encoding()
            );
        }
        else
        {
            unsigned int wday = wd.c_encoding();
            PAPILIO_ASSERT(wday < 7);
            if(fullname)
            {
                std::basic_string_view<CharT> sv = weekday_names_full<CharT>[wday];
                return std::copy(sv.begin(), sv.end(), out);
            }
            else
                return std::copy_n(weekday_names_short<CharT>[wday], 3, out);
        }
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

    template <typename CharT, typename OutputIt>
    OutputIt put_month_by_name(OutputIt out, const std::chrono::month& m, bool fullname)
    {
        if(!m.ok()) [[unlikely]]
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "month({})"),
                static_cast<unsigned int>(m)
            );
        }
        else
        {
            unsigned int mon = static_cast<unsigned int>(m) - 1;
            PAPILIO_ASSERT(mon < 12);
            if(fullname)
            {
                std::basic_string_view<CharT> sv = month_names_full<CharT>[mon];
                return std::copy(sv.begin(), sv.end(), out);
            }
            else
                return std::copy_n(month_names_short<CharT>[mon], 3, out);
        }
    }

    template <typename CharT, typename Period, typename OutputIt>
    OutputIt put_time_unit_suffix(OutputIt out)
    {
        auto helper = [&out](std::string_view sv) -> OutputIt
        {
            return std::copy(sv.begin(), sv.end(), out);
        };

        using type = typename Period::type;
        using std::is_same_v;
        if constexpr(is_same_v<type, std::atto>)
            return helper("as");
        else if constexpr(is_same_v<type, std::femto>)
            return helper("fs");
        else if constexpr(is_same_v<type, std::pico>)
            return helper("ps");
        else if constexpr(is_same_v<type, std::nano>)
            return helper("ns");
        else if constexpr(is_same_v<type, std::micro>)
            return helper("us");
        else if constexpr(is_same_v<type, std::milli>)
            return helper("ms");
        else if constexpr(is_same_v<type, std::centi>)
            return helper("cs");
        else if constexpr(is_same_v<type, std::deci>)
            return helper("ds");
        else if constexpr(is_same_v<type, std::ratio<1>>)
            return helper("s");
        else if constexpr(is_same_v<type, std::deca>)
            return helper("das");
        else if constexpr(is_same_v<type, std::hecto>)
            return helper("hs");
        else if constexpr(is_same_v<type, std::kilo>)
            return helper("ks");
        else if constexpr(is_same_v<type, std::mega>)
            return helper("Ms");
        else if constexpr(is_same_v<type, std::giga>)
            return helper("Gs");
        else if constexpr(is_same_v<type, std::tera>)
            return helper("Ts");
        else if constexpr(is_same_v<type, std::peta>)
            return helper("Ps");
        else if constexpr(is_same_v<type, std::exa>)
            return helper("Es");
        else if constexpr(is_same_v<type, std::ratio<60>>)
            return helper("min");
        else if constexpr(is_same_v<type, std::ratio<3600>>)
            return helper("h");
        else if constexpr(is_same_v<type, std::ratio<86400>>)
            return helper("d");
        else if constexpr(Period::type::den == 1)
        {
            return PAPILIO_NS format_to(
                out, PAPILIO_TSTRING_VIEW(CharT, "[{}]s"), Period::type::num
            );
        }
        else
        {
            return PAPILIO_NS format_to(
                out, PAPILIO_TSTRING_VIEW(CharT, "[{}/{}]s"), Period::type::num, Period::type::den
            );
        }
    }

    template <typename CharT, typename ChronoType, typename OutputIt>
    OutputIt put_count(OutputIt out, const ChronoType& val, bool use_unit)
    {
        out = PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{}"),
            val.count()
        );
        if(!use_unit)
            return out;

        return detail::put_time_unit_suffix<CharT, typename ChronoType::period>(
            out
        );
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

    /**
     * @brief Time zone information needed for formatting
     */
    struct timezone_info
    {
        std::string abbrev;
        std::chrono::seconds offset;
    };

    template <typename ChronoType>
    timezone_info get_timezone_info([[maybe_unused]] const ChronoType& val)
    {
        using std::is_same_v;

#ifndef PAPILIO_CHRONO_NO_TIMEZONE
        if constexpr(is_same_v<ChronoType, std::chrono::sys_info>)
            return {val.abbrev, val.offset};
#endif

        return {"UTC", std::chrono::seconds(0)};
    }

    template <typename CharT, typename OutputIt>
    OutputIt put_timezone_abbrev(OutputIt out, const timezone_info& tz)
    {
        // The abbreviation always consists of char, so use copy() to convert
        return std::copy(tz.abbrev.begin(), tz.abbrev.end(), out);
    }

    template <typename CharT, typename OutputIt>
    OutputIt put_timezone_offset(OutputIt out, std::chrono::seconds offset, bool alt_fmt)
    {
        if(offset < std::chrono::seconds(0))
        {
            *out = CharT('-');
            ++out;
            offset = -offset;
        }
        else
        {
            *out = CharT('+');
            ++out;
        }

        std::chrono::hh_mm_ss hms(offset);
        if(alt_fmt)
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{:02}:{:02}"),
                hms.hours().count(),
                hms.minutes().count()
            );
        }
        else
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{:02}{:02}"),
                hms.hours().count(),
                hms.minutes().count()
            );
        }
    }
} // namespace detail

template <typename ChronoType, typename CharT>
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
            return fmt.format(
                default_impl(val),
                ctx
            );
        }
        else if(!m_data.basic.use_locale)
        {
            return fmt.format(
                to_str(val, m_data.chrono_spec),
                ctx
            );
        }
        else
        {
            return fmt.format(
                to_str_by_facet(ctx.getloc(), val, m_data.chrono_spec),
                ctx
            );
        }
    }

private:
    chrono_formatter_data<CharT> m_data;

    static std::basic_string<CharT> default_impl(const ChronoType& val)
    {
        using std::is_same_v;
        using namespace std::chrono;

        std::basic_string<CharT> result;

        if constexpr(is_specialization_of_v<ChronoType, duration>)
        {
            detail::put_count<CharT>(
                std::back_inserter(result), val, true
            );
        }
        else if constexpr(is_specialization_of_v<ChronoType, time_point>)
        {
            result = to_str(val, PAPILIO_TSTRING_VIEW(CharT, "%F %T"));
        }
        else if constexpr(is_same_v<ChronoType, year>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "{:04d}"),
                static_cast<int>(val)
            );
        }
        else if constexpr(is_same_v<ChronoType, month>)
        {
            detail::put_month_by_name<CharT>(
                std::back_inserter(result),
                val,
                false
            );
        }
        else if constexpr(is_same_v<ChronoType, day>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                static_cast<unsigned int>(val)
            );
        }
        else if constexpr(is_specialization_of_v<ChronoType, hh_mm_ss>)
        {
            result = to_str(val, PAPILIO_TSTRING_VIEW(CharT, "%T"));
        }
        else if constexpr(is_same_v<ChronoType, year_month_day>)
        {
            result = to_str(val, PAPILIO_TSTRING_VIEW(CharT, "%F"));
        }
        else if constexpr(is_same_v<ChronoType, year_month>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "{}/{}"),
                val.year(),
                val.month()
            );
        }
        else if constexpr(is_same_v<ChronoType, month_day>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "{}/{}"),
                val.month(),
                val.day()
            );
        }
        else if constexpr(is_same_v<ChronoType, weekday>)
        {
            detail::put_weekday_by_name<CharT>(
                std::back_inserter(result),
                val,
                false
            );
        }
        else if constexpr(is_same_v<ChronoType, weekday_indexed>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "{}[{}]"),
                val.weekday(),
                val.index()
            );
        }
        else if constexpr(is_same_v<ChronoType, weekday_last>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "{}[last]"),
                val.weekday()
            );
        }
#ifndef PAPILIO_CHRONO_NO_TIMEZONE
        else if constexpr(is_same_v<ChronoType, sys_info>)
        {
            result = PAPILIO_NS format(
                PAPILIO_TSTRING_VIEW(CharT, "({}, {}, {}, {}, {})"),
                val.begin,
                val.end,
                val.offset,
                val.save,
                val.abbrev
            );
        }
#endif

        return result;
    }

    static std::basic_string<CharT> to_str(const ChronoType& val, utf::basic_string_ref<CharT> spec)
    {
        std::tm t = chrono_traits_type::to_tm(val);
        detail::timezone_info tz = PAPILIO_NS detail::get_timezone_info(val);

        static constexpr bool has_count = requires() {
            val.count();
            typename ChronoType::period;
        };

        const auto sentinel = spec.end();

        std::basic_stringstream<CharT> ss;
        for(auto it = spec.begin(); it != sentinel; ++it)
        {
            utf::codepoint ch = *it;
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
                detail::format_century(ss, t.tm_year + 1900);
                continue;

            case U'y':
                detail::format_year(ss, t.tm_year + 1900, false);
                continue;
            case U'Y':
                detail::format_year(ss, t.tm_year + 1900, true);
                continue;

            case U'm':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                    t.tm_mon + 1
                );
                continue;
            case U'b':
            case U'B':
                detail::put_month_by_name<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    std::chrono::month(t.tm_mon + 1),
                    ch32 == U'B'
                );
                continue;

            case U'd':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                    t.tm_mday
                );
                continue;
            case U'e':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:2d}"),
                    t.tm_mday
                );
                continue;

                // Day of the week
            case U'u':
                detail::format_weekday(ss, t.tm_wday, true);
                continue;
            case U'w':
                detail::format_weekday(ss, t.tm_wday, false);
                continue;
            case U'a':
            case U'A':
                detail::put_weekday_by_name<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    std::chrono::weekday(t.tm_wday),
                    ch32 == U'A'
                );
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
                detail::put_hour<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    t.tm_hour,
                    ch32 == U'I'
                );
                continue;

            case U'M':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                    t.tm_min
                );
                continue;

            case U'S':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
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

            case 'z':
                PAPILIO_NS detail::put_timezone_offset<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    tz.offset,
                    false
                );
                continue;
            case 'Z':
                PAPILIO_NS detail::put_timezone_abbrev<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    tz
                );
                continue;

            case U'R':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}:{:02d}"),
                    t.tm_hour,
                    t.tm_min
                );
                continue;
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

            case U'r': // Equivalent to %I:%M:%S %p
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
                detail::put_am_pm<CharT>(
                    std::ostreambuf_iterator<CharT>(ss),
                    std::chrono::is_am(std::chrono::hours(t.tm_hour))
                );
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
                PAPILIO_NS detail::put_asctime(
                    std::ostreambuf_iterator<CharT>(ss), t
                );
                continue;

            case U'q':
            case U'Q':
                if constexpr(has_count)
                {
                    detail::put_count<CharT>(
                        std::ostreambuf_iterator<CharT>(ss), val, ch32 == U'q'
                    );
                }
                else
                {
                    throw_bad_format("count() is not supported for this type");
                }
                continue;

            case U'O':
            case U'E':
                if(it == sentinel)
                {
                    throw_bad_format("invalid format for locale");
                }
                else if(char32_t loc_ch = *std::next(it); loc_ch == U'z')
                {
                    PAPILIO_NS detail::put_timezone_offset<CharT>(
                        std::ostreambuf_iterator<CharT>(ss),
                        tz.offset,
                        true
                    );
                    ++it;
                }
                else
                {
                    throw_bad_format("locale is not supported without 'L'");
                }
                continue;

            default:
                {
                    std::string msg = "unsupported specifier %";
                    ch.append_to(msg);
                    throw_bad_format(msg.c_str());
                }
            }
        }

        return std::move(ss).str();
    }

    static std::basic_string<CharT> to_str_by_facet(const std::locale& loc, const ChronoType& val, std::basic_string_view<CharT> spec)
    {
        auto& facet = std::use_facet<std::time_put<CharT>>(loc);
        std::basic_stringstream<CharT> ss;
        ss.imbue(loc);

        std::tm t = chrono_traits_type::to_tm(val);
        facet.put(
            std::ostreambuf_iterator<CharT>(ss),
            ss,
            static_cast<CharT>(' '),
            &t,
            spec.data(),
            spec.data() + spec.size()
        );

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

template <typename Duration, typename CharT>
class formatter<std::chrono::sys_time<Duration>, CharT> :
    public chrono_formatter<std::chrono::sys_time<Duration>, CharT>
{};

#ifndef PAPILIO_CHRONO_NO_UTC_TIME

template <typename Duration, typename CharT>
class formatter<std::chrono::utc_time<Duration>, CharT> :
    public chrono_formatter<std::chrono::utc_time<Duration>, CharT>
{};

#endif

template <typename Rep, typename Period, typename CharT>
class formatter<std::chrono::duration<Rep, Period>, CharT> :
    public chrono_formatter<std::chrono::duration<Rep, Period>, CharT>
{};

template <typename CharT>
class formatter<std::chrono::year_month, CharT> :
    public chrono_formatter<std::chrono::year_month, CharT>
{};

template <typename CharT>
class formatter<std::chrono::month_day, CharT> :
    public chrono_formatter<std::chrono::month_day, CharT>
{};

template <typename CharT>
class formatter<std::chrono::year_month_day, CharT> :
    public chrono_formatter<std::chrono::year_month_day, CharT>
{};

template <typename CharT>
class formatter<std::chrono::year_month_day_last, CharT> :
    public chrono_formatter<std::chrono::year_month_day_last, CharT>
{};

template <typename CharT>
class formatter<std::chrono::year, CharT> :
    public chrono_formatter<std::chrono::year, CharT>
{};

template <typename CharT>
class formatter<std::chrono::month, CharT> :
    public chrono_formatter<std::chrono::month, CharT>
{};

template <typename CharT>
class formatter<std::chrono::day, CharT> :
    public chrono_formatter<std::chrono::day, CharT>
{};

template <typename CharT>
class formatter<std::chrono::weekday, CharT> :
    public chrono_formatter<std::chrono::weekday, CharT>
{};

template <typename CharT>
class formatter<std::chrono::weekday_indexed, CharT> :
    public chrono_formatter<std::chrono::weekday_indexed, CharT>
{};

template <typename CharT>
class formatter<std::chrono::weekday_last, CharT> :
    public chrono_formatter<std::chrono::weekday_last, CharT>
{};

template <typename Duration, typename CharT>
class formatter<std::chrono::hh_mm_ss<Duration>, CharT> :
    public chrono_formatter<std::chrono::hh_mm_ss<Duration>, CharT>
{};

#ifndef PAPILIO_CHRONO_NO_TIMEZONE

template <typename CharT>
class formatter<std::chrono::sys_info, CharT> :
    public chrono_formatter<std::chrono::sys_info, CharT>
{};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
