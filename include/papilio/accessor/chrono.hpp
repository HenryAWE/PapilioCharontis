#ifndef PAPILIO_ACCESSOR_CHRONO_HPP
#define PAPILIO_ACCESSOR_CHRONO_HPP

#include <ctime>
#include <chrono>
#include "../access.hpp"
#include "../chrono/chrono_traits.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename Context>
struct accessor<std::tm, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    static format_arg_type attribute(const std::tm& val, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "year"))
            return val.tm_year + 1900;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "month"))
            return val.tm_mon;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "mday"))
            return val.tm_mday;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "hour"))
            return val.tm_hour;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "min"))
            return val.tm_min;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "sec"))
            return val.tm_sec;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "wday"))
            return val.tm_wday;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "yday"))
            return val.tm_yday;
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "is_dst"))
            return static_cast<bool>(val.tm_isdst);

        throw_invalid_attribute(attr);
    }
};

template <chrono::chrono_type ChronoType, typename Context>
struct accessor<ChronoType, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    static format_arg_type attribute(const ChronoType& val, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "ok"))
        {
            if constexpr(requires() { {val.ok() } -> std::convertible_to<bool>; })
                return static_cast<bool>(val.ok());
            else
                return true;
        }

        if constexpr(requires() { val.year(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "year"))
                return val.year();
        }
        if constexpr(requires() { val.month(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "month"))
                return val.month();
        }
        if constexpr(requires() { val.day(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "day"))
                return val.day();
        }

        if constexpr(requires() { val.weekday(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "weekday"))
                return val.weekday();
        }
        else if constexpr(std::is_constructible_v<std::chrono::weekday, const ChronoType&> &&
                          !std::same_as<ChronoType, std::chrono::weekday>)
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "weekday"))
                return std::chrono::weekday(val);
        }

        if constexpr(requires() { val.hours(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "hour"))
                return val.hours();
        }
        if constexpr(requires() { val.minutes(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "minute"))
                return val.minutes();
        }
        if constexpr(requires() { val.seconds(); })
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "second"))
                return val.seconds();
        }

        throw_invalid_attribute(attr);
    }
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
