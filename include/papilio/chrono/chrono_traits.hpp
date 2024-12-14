#ifndef PAPILIO_CHRONO_CHRONO_TRAITS_HPP
#define PAPILIO_CHRONO_CHRONO_TRAITS_HPP

#pragma once

#include <ctime>
#include <chrono>
#include <type_traits>
#include "../format.hpp"
#include "chrono_utility.hpp"
#include "../detail/prefix.hpp"

namespace papilio::chrono
{
enum class components : int
{
    none = 0,

    day = 1 << 0,
    month = 1 << 1,
    year = 1 << 2,

    date = year | month | day,

    hour_min_sec = 1 << 3,

    date_time = date | hour_min_sec,

    weekday = 1 << 4,

    duration_count = 1 << 5,

    time_zone = 1 << 6,

    all = date_time | weekday | time_zone
};

constexpr components operator|(components lhs, components rhs) noexcept
{
    return components(PAPILIO_NS to_underlying(lhs) | PAPILIO_NS to_underlying(rhs));
}

constexpr components operator&(components lhs, components rhs) noexcept
{
    return components(PAPILIO_NS to_underlying(lhs)& PAPILIO_NS to_underlying(rhs));
}

constexpr bool operator!(components val) noexcept
{
    return !static_cast<bool>(val);
}

#if defined(PAPILIO_COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wsign-conversion"
#endif

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
} // namespace detail

template <typename ChronoType>
struct chrono_traits
{
    static constexpr components get_components() noexcept
    {
        return components::none;
    }

    static std::tm to_tm(const ChronoType&)
    {
        return detail::init_tm();
    }
};

template <>
struct chrono_traits<std::chrono::year>
{
    static constexpr components get_components()
    {
        return components::year;
    }

    static std::tm to_tm(const std::chrono::year& y)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();
        result.tm_year = static_cast<int>(y) - 1900;
        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::year& y)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{:04d}"),
            static_cast<int>(y)
        );
    }
};

template <>
struct chrono_traits<std::chrono::month>
{
    static constexpr components get_components()
    {
        return components::month;
    }

    static std::tm to_tm(const std::chrono::month& m)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();
        result.tm_mon = static_cast<unsigned int>(m) - 1;
        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::month& m)
    {
        return PAPILIO_NS chrono::copy_month_name<CharT>(out, m);
    }
};

template <>
struct chrono_traits<std::chrono::day>
{
    static constexpr components get_components() noexcept
    {
        return components::day;
    }

    static std::tm to_tm(const std::chrono::day& d)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();
        result.tm_mday = static_cast<unsigned int>(d);
        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::day& d)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{:02d}"),
            static_cast<unsigned int>(d)
        );
    }
};

template <>
struct chrono_traits<std::chrono::year_month>
{
    static constexpr components get_components() noexcept
    {
        return components::year | components::month;
    }

    static std::tm to_tm(const std::chrono::year_month& ym)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();

        result.tm_year = static_cast<int>(ym.year()) - 1900;
        result.tm_mon = static_cast<unsigned int>(ym.month()) - 1;

        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::year_month& ym)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{}/{}"),
            ym.year(),
            ym.month()
        );
    }
};

template <>
struct chrono_traits<std::chrono::month_day>
{
    static constexpr components get_components() noexcept
    {
        return components::month | components::day;
    }

    static std::tm to_tm(const std::chrono::month_day& md)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();

        result.tm_mon = static_cast<unsigned int>(md.month()) - 1;
        result.tm_mday = static_cast<unsigned int>(md.day());

        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::month_day& md)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{}/{}"),
            md.month(),
            md.day()
        );
    }
};

template <>
struct chrono_traits<std::chrono::month_day_last> : public chrono_traits<std::chrono::month_day>
{
    // We cannot calculate the exact day without knowing the year.
    // For example, Feb/last can be Feb/28 or Feb/29

    static constexpr components get_components() noexcept
    {
        return components::month;
    }

    static std::tm to_tm(const std::chrono::month_day_last& md)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();

        result.tm_mon = static_cast<unsigned int>(md.month()) - 1;

        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::month_day_last& md)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{}/last"),
            md.month()
        );
    }
};

template <>
struct chrono_traits<std::chrono::year_month_day>
{
    static constexpr components get_components()
    {
        return components::date;
    }

    static std::tm to_tm(const std::chrono::year_month_day& date)
    {
        namespace chrono = std::chrono;

        std::tm result = PAPILIO_NS chrono::detail::init_tm();

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

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::year_month_day& ymd)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{:%F}"),
            ymd
        );
    }
};

template <>
struct chrono_traits<std::chrono::year_month_day_last> : public chrono_traits<std::chrono::year_month_day>
{
    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::year_month_day_last& ymd)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{}/{}"),
            ymd.year(),
            ymd.month_day_last()
        );
    }
};

template <typename Duration>
struct chrono_traits<std::chrono::sys_time<Duration>>
{
    static constexpr components get_components() noexcept
    {
        return components::all;
    }

    static std::tm to_tm(const std::chrono::sys_time<Duration>& t)
    {
        namespace chrono = std::chrono;

        chrono::sys_days days = chrono::floor<chrono::days>(t);
        chrono::year_month_day ymd(days);

        std::tm result = chrono_traits<std::chrono::year_month_day>::to_tm(ymd);

        std::int64_t sec = chrono::duration_cast<chrono::seconds>(t - chrono::time_point_cast<chrono::seconds>(days)).count();
        sec %= 24 * 3600;
        result.tm_hour = static_cast<int>(sec / 3600);
        sec %= 3600;
        result.tm_min = static_cast<int>(sec / 60);
        result.tm_sec = static_cast<int>(sec % 60);

        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::sys_time<Duration>& t)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{:%F %T}"),
            t
        );
    }
};

template <typename Clock, typename Duration>
struct chrono_traits<std::chrono::time_point<Clock, Duration>>
{
    static constexpr components get_components() noexcept
    {
        return components::all;
    }

    static std::tm to_tm(const std::chrono::time_point<Clock, Duration>& t)
    {
        if constexpr(std::is_same_v<Clock, std::chrono::file_clock>)
        {
            auto sys = Clock::to_sys(t);
            return chrono_traits<decltype(sys)>::to_tm(sys);
        }
        else if(std::is_same_v<Clock, std::chrono::local_t>)
        {
            return chrono_traits<std::chrono::sys_time<Duration>>(
                to_tm(std::chrono::sys_time<Duration>(t.time_since_epoch()))
            );
        }
        else
        {
            static_assert(!sizeof(Clock), "invalid clock type");
        }
    }
};

template <typename Rep, typename Period>
struct chrono_traits<std::chrono::duration<Rep, Period>>
{
    static constexpr components get_components() noexcept
    {
        return components::hour_min_sec | components::duration_count;
    }

    static std::tm to_tm(const std::chrono::duration<Rep, Period>& d)
    {
        namespace chrono = std::chrono;

        std::tm result = PAPILIO_NS chrono::detail::init_tm();

        std::int64_t sec = chrono::duration_cast<chrono::seconds>(d).count();
        sec %= 24 * 3600;
        result.tm_hour = static_cast<int>(sec / 3600);
        sec %= 3600;
        result.tm_min = static_cast<int>(sec / 60);
        result.tm_sec = static_cast<int>(sec % 60);

        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::duration<Rep, Period>& d)
    {
        return PAPILIO_NS chrono::copy_count(out, d);
    }
};

template <>
struct chrono_traits<std::chrono::weekday>
{
    static constexpr components get_components() noexcept
    {
        return components::weekday;
    }

    static std::tm to_tm(const std::chrono::weekday& wd)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();
        result.tm_wday = wd.c_encoding();
        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::weekday& wd)
    {
        return PAPILIO_NS chrono::copy_weekday_name<CharT>(out, wd);
    }
};

template <typename WeekdayType>
requires(std::same_as<WeekdayType, std::chrono::weekday_last> || std::same_as<WeekdayType, std::chrono::weekday_indexed>)
struct chrono_traits<WeekdayType> : public chrono_traits<std::chrono::weekday>
{
    static std::tm to_tm(const WeekdayType& wd)
    {
        return chrono_traits<std::chrono::weekday>::to_tm(wd.weekday());
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const WeekdayType& wd)
    {
        if constexpr(std::same_as<WeekdayType, std::chrono::weekday_indexed>)
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{}[{}]"),
                wd.weekday(),
                wd.index()
            );
        }
        else // std::chrono::weekday_last
        {
            return PAPILIO_NS format_to(
                out,
                PAPILIO_TSTRING_VIEW(CharT, "{}[last]"),
                wd.weekday()
            );
        }
    }
};

template <typename Duration>
struct chrono_traits<std::chrono::hh_mm_ss<Duration>>
{
    static constexpr components get_components() noexcept
    {
        return components::hour_min_sec;
    }

    static std::tm to_tm(const std::chrono::hh_mm_ss<Duration>& t)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();
        result.tm_hour = t.hours().count();
        result.tm_min = t.minutes().count();
        result.tm_sec = static_cast<int>(t.seconds().count());
        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::hh_mm_ss<Duration>& hms)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "{:%T}"),
            hms
        );
    }
};

#ifndef PAPILIO_CHRONO_NO_TIMEZONE

template <>
struct chrono_traits<std::chrono::sys_info>
{
    static constexpr components get_components() noexcept
    {
        return components::time_zone;
    }

    static timezone_info get_timezone_info(const std::chrono::sys_info& info) noexcept
    {
        return {info.abbrev, info.offset};
    }

    static std::tm to_tm([[maybe_unused]] const std::chrono::sys_info& info)
    {
        std::tm result = PAPILIO_NS chrono::detail::init_tm();

#    ifdef __GLIBC__
        result.tm_gmtoff = info.offset.count();
        result.tm_zone = info.abbrev.empty() ? "" : info.abbrev.c_str();
#    endif

        return result;
    }

    template <typename CharT, typename OutputIt>
    static OutputIt default_format(locale_ref, OutputIt out, const std::chrono::sys_info& info)
    {
        return PAPILIO_NS format_to(
            out,
            PAPILIO_TSTRING_VIEW(CharT, "({}, {}, {}, {}, {})"),
            info.begin,
            info.end,
            info.offset,
            info.save,
            info.abbrev
        );
    }
};

#endif

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

template <typename T>
concept chrono_type =
    chrono_traits<T>::get_components() != components::none;

template <chrono_type ChronoType>
timezone_info get_timezone_info(const ChronoType& val)
{
    using std::is_same_v;

    using chrono_traits_type = chrono_traits<ChronoType>;

    constexpr bool has_get_timezone_info = requires() {
        { chrono_traits_type::get_timezone_info(val) } -> std::convertible_to<timezone_info>;
    };
    if constexpr(has_get_timezone_info)
    {
        return chrono_traits_type::get_timezone_info(val);
    }

    return {"UTC", std::chrono::seconds(0)};
}
} // namespace papilio::chrono

#include "../detail/suffix.hpp"

#endif
