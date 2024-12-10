#ifndef PAPILIO_FORMATTER_CHRONO_HPP
#define PAPILIO_FORMATTER_CHRONO_HPP

#pragma once

#include <ctime>
#include <span>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "../format.hpp"
#include "../detail/prefix.hpp"

// Workarounds
#ifdef PAPILIO_STDLIB_LIBCPP
#    define PAPILIO_CHRONO_NO_UTC_TIME
#endif

namespace papilio
{
PAPILIO_EXPORT template <typename CharT>
struct chrono_formatter_data
{
    using string_container_type = utf::basic_string_container<CharT>;

    simple_formatter_data basic;
    string_container_type chrono_spec;
};

template <typename ParseContext>
PAPILIO_EXPORT class chrono_formatter_parser
{
public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = chrono_formatter_data<char_type>;
    using interpreter_type = basic_interpreter<typename ParseContext::format_context_type>;

    static std::pair<result_type, iterator> parse(ParseContext& ctx)
    {
        result_type result;

        simple_formatter_parser<ParseContext, true> basic_parser;
        auto [basic_result, it] = basic_parser.parse(ctx);
        result.basic = basic_result;

        auto spec_end = std::find_if(
            it,
            ctx.end(),
            [](char32_t v)
            { return v == U'}'; }
        );
        result.chrono_spec.assign(it, spec_end);

        return std::make_pair(std::move(result), spec_end);
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
    inline std::tm init_tm() noexcept
    {
        std::tm init{};

#ifdef __GLIBC__
        init.tm_zone = "UTC";
#endif

        return init;
    }

    inline std::tm to_tm(const std::chrono::year_month_day& date)
    {
        namespace chrono = std::chrono;

        std::tm result = init_tm();

        result.tm_year = static_cast<int>(date.year()) - 1900;
        result.tm_mon = static_cast<unsigned int>(date.month()) - 1;
        result.tm_mday = static_cast<unsigned int>(date.day());
        result.tm_wday = static_cast<unsigned int>(std::chrono::weekday(date).c_encoding());
        {
            auto yday =
                static_cast<std::chrono::sys_days>(date) -
                static_cast<std::chrono::sys_days>(chrono::year_month_day{date.year(), chrono::January, chrono::day(1)});
            result.tm_yday = yday.count();
        }

        return result;
    }

    inline std::tm to_tm(const std::chrono::year_month_day_last& date)
    {
        return to_tm(std::chrono::year_month_day(date));
    }

    inline std::tm to_tm(const std::chrono::year_month& date)
    {
        std::tm result = init_tm();

        result.tm_year = static_cast<int>(date.year()) - 1900;
        result.tm_mon = static_cast<unsigned int>(date.month()) - 1;

        return result;
    }

    inline std::tm to_tm(const std::chrono::month_day& date)
    {
        std::tm result = init_tm();

        result.tm_mon = static_cast<unsigned int>(date.month()) - 1;
        result.tm_mday = static_cast<unsigned int>(date.day());

        return result;
    }

    inline std::tm to_tm(const std::chrono::year_month_day& date, std::chrono::weekday weekday)
    {
        namespace chrono = std::chrono;

        std::tm result = init_tm();

        result.tm_year = static_cast<int>(date.year()) - 1900;
        result.tm_mon = static_cast<unsigned int>(date.month()) - 1;
        result.tm_mday = static_cast<unsigned int>(date.day());
        result.tm_wday = static_cast<unsigned int>(weekday.c_encoding());
        {
            auto yday =
                static_cast<std::chrono::sys_days>(date) -
                static_cast<std::chrono::sys_days>(chrono::year_month_day{date.year(), chrono::January, chrono::day(1)});
            result.tm_yday = yday.count();
        }

        return result;
    }

    template <typename Duration>
    std::tm to_tm(const std::chrono::sys_time<Duration>& t)
    {
        namespace chrono = std::chrono;

        chrono::sys_days days = chrono::floor<chrono::days>(t);
        chrono::year_month_day ymd(days);

        std::tm result = to_tm(ymd, chrono::weekday(days));

        std::int64_t sec = chrono::duration_cast<chrono::seconds>(t - chrono::time_point_cast<chrono::seconds>(days)).count();
        sec %= 24 * 3600;
        result.tm_hour = static_cast<int>(sec / 3600);
        sec %= 3600;
        result.tm_min = static_cast<int>(sec / 60);
        result.tm_sec = static_cast<int>(sec % 60);

        return result;
    }

    template <typename Clock, typename Duration>
    std::tm to_tm(const std::chrono::time_point<Clock, Duration>& t)
    {
        namespace chrono = std::chrono;
        if constexpr(std::is_same_v<Clock, chrono::file_clock>)
        {
            return to_tm(Clock::to_sys(t));
        }
        else if(std::is_same_v<Clock, chrono::local_t>)
        {
            return to_tm(chrono::sys_time<Duration>(t.time_since_epoch()));
        }
        else
        {
            static_assert(!sizeof(Clock), "invalid clock type");
        }
    }

    template <typename Rep, typename Period>
    std::tm to_tm(const std::chrono::duration<Rep, Period>& d)
    {
        namespace chrono = std::chrono;

        std::tm result = init_tm();

        std::int64_t sec = chrono::duration_cast<chrono::seconds>(d).count();
        sec %= 24 * 3600;
        result.tm_hour = static_cast<int>(sec / 3600);
        sec %= 3600;
        result.tm_min = static_cast<int>(sec / 60);
        result.tm_sec = static_cast<int>(sec % 60);

        return result;
    }

    inline std::tm to_tm(const std::chrono::year& y)
    {
        std::tm result = init_tm();
        result.tm_year = static_cast<int>(y) - 1900;
        return result;
    }

    inline std::tm to_tm(const std::chrono::month& m)
    {
        std::tm result = init_tm();
        result.tm_mon = static_cast<unsigned int>(m) - 1;
        return result;
    }

    inline std::tm to_tm(const std::chrono::day& d)
    {
        std::tm result = init_tm();
        result.tm_mday = static_cast<unsigned int>(d);
        return result;
    }

    inline std::tm to_tm(const std::chrono::weekday& wd)
    {
        std::tm result = init_tm();
        result.tm_wday = wd.c_encoding();
        return result;
    }

    inline std::tm to_tm(const std::chrono::weekday_indexed& wd)
    {
        std::tm result = init_tm();
        result.tm_wday = wd.weekday().c_encoding();
        return result;
    }

    inline std::tm to_tm(const std::chrono::weekday_last& wd)
    {
        std::tm result = init_tm();
        result.tm_wday = wd.weekday().c_encoding();
        return result;
    }

    template <typename Duration>
    inline std::tm to_tm(const std::chrono::hh_mm_ss<Duration>& t)
    {
        std::tm result = init_tm();
        result.tm_hour = t.hours().count();
        result.tm_min = t.minutes().count();
        result.tm_sec = static_cast<int>(t.seconds().count());
        return result;
    }

    inline std::tm to_tm(const std::chrono::sys_info&)
    {
        return init_tm();
    }

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
    timezone_info get_timezone_info(const ChronoType& val)
    {
        using std::is_same_v;
        if constexpr(is_same_v<ChronoType, std::chrono::sys_info>)
            return {val.abbrev, val.offset};

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

        return result;
    }

    static std::basic_string<CharT> to_str(const ChronoType& val, utf::basic_string_ref<CharT> spec)
    {
        std::tm t = detail::to_tm(val);
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
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                    t.tm_hour
                );
                continue;
            case U'I':
                PAPILIO_NS format_to(
                    std::ostreambuf_iterator<CharT>(ss),
                    PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
                    t.tm_hour % 12
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

        std::tm t = detail::to_tm(val);
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

template <typename CharT>
class formatter<std::chrono::sys_info, CharT> :
    public chrono_formatter<std::chrono::sys_info, CharT>
{};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
