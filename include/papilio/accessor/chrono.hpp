#ifndef PAPILIO_ACCESSOR_CHRONO_HPP
#define PAPILIO_ACCESSOR_CHRONO_HPP

#include <ctime>
#include <chrono>
#include "../access.hpp"
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
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
