#ifndef PAPILIO_CHRONO_CHRONO_UTILITY_HPP
#define PAPILIO_CHRONO_CHRONO_UTILITY_HPP

#pragma once

#include <ctime>
#include <chrono>
#include <type_traits>
#include <algorithm>
#include "../utility.hpp"
#include "../format.hpp"
#include "../detail/prefix.hpp"

// Workarounds
#ifdef PAPILIO_STDLIB_LIBCPP
#    define PAPILIO_CHRONO_NO_UTC_TIME
#endif

#ifdef PAPILIO_STDLIB_LIBCPP
#    define PAPILIO_CHRONO_NO_TIMEZONE
#elif defined(PAPILIO_STDLIB_LIBSTDCPP)
#    if __GLIBCXX__ < 20240412
#        define PAPILIO_CHRONO_NO_TIMEZONE
#    endif
#endif

namespace papilio::chrono
{

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
inline constexpr std::basic_string_view<CharT> weekday_names_full[7] = {
    PAPILIO_TSTRING_VIEW(CharT, "Sunday"),
    PAPILIO_TSTRING_VIEW(CharT, "Monday"),
    PAPILIO_TSTRING_VIEW(CharT, "Tuesday"),
    PAPILIO_TSTRING_VIEW(CharT, "Wednesday"),
    PAPILIO_TSTRING_VIEW(CharT, "Thursday"),
    PAPILIO_TSTRING_VIEW(CharT, "Friday"),
    PAPILIO_TSTRING_VIEW(CharT, "Saturday"),
};

template <typename CharT = char, typename OutputIt>
OutputIt copy_weekday_name(OutputIt out, const std::chrono::weekday& wd, bool fullname = false)
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

template <typename CharT = char, typename OutputIt>
OutputIt copy_month_name(OutputIt out, const std::chrono::month& m, bool fullname = false)
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

/**
    * @brief Similar to `std::asctime()` but without the trailing newline
    *
    * @param out Output iterator
    * @param t Time value
    */
template <typename CharT = char, typename OutputIt>
OutputIt copy_asctime(OutputIt out, const std::tm& t)
{
    int wday = std::clamp(t.tm_wday, 0, 6);
    int mon = std::clamp(t.tm_mon, 0, 11);

    return PAPILIO_NS format_to(
        out,
        PAPILIO_TSTRING_VIEW(CharT, "{} {} {:2d} {:02d}:{:02d}:{:02d} {:4d}"),
        std::basic_string_view<CharT>(chrono::weekday_names_short<CharT>[wday], 3),
        std::basic_string_view<CharT>(chrono::month_names_short<CharT>[mon], 3),
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec,
        t.tm_year + 1900
    );
}

template <typename CharT = char, typename Period, typename OutputIt>
OutputIt copy_unit_suffix(OutputIt out, std::in_place_type_t<Period>)
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

template <typename CharT = char, typename ChronoType, typename OutputIt>
requires(is_specialization_of_v<ChronoType, std::chrono::duration>)
OutputIt copy_count(OutputIt out, const ChronoType& val, bool use_unit = true)
{
    out = PAPILIO_NS format_to(
        out,
        PAPILIO_TSTRING_VIEW(CharT, "{}"),
        val.count()
    );
    if(!use_unit)
        return std::move(out);

    return PAPILIO_NS chrono::copy_unit_suffix<CharT>(
        out,
        std::in_place_type<typename ChronoType::period>
    );
}

/**
* @brief Time zone information needed for formatting.
*/
struct timezone_info
{
    std::string abbrev;
    std::chrono::seconds offset;

    template <typename OutputIt>
    OutputIt copy_abbrev(OutputIt out) const
    {
        // The abbreviation always consists of char, so use copy() to convert
        return std::copy(abbrev.begin(), abbrev.end(), out);
    }

    template <typename CharT, typename OutputIt>
    OutputIt copy_offset(OutputIt out, bool alt_fmt = false) const
    {
        std::chrono::seconds val = offset;
        if(val < std::chrono::seconds(0))
        {
            *out = CharT('-');
            ++out;
            val = -val;
        }
        else
        {
            *out = CharT('+');
            ++out;
        }

        std::chrono::hh_mm_ss hms(val);
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
};
} // namespace papilio::chrono

#include "../detail/suffix.hpp"

#endif
