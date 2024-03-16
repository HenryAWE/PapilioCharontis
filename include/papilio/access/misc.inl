#include <string_view>
#include <typeindex>
#include "../utility.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename Context>
struct accessor<std::type_index, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    static format_arg_type attribute(std::type_index info, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "name"))
        {
            if constexpr(char8_like<char_type>)
            {
                return std::bit_cast<const char_type*>(info.name());
            }
            else
            {
                utf::string_ref narrow_name = info.name();
                return narrow_name.to_string_as<char_type>();
            }
        }
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "hash_code"))
        {
            return info.hash_code();
        }

        throw_invalid_attribute(attr);
    }
};
} // namespace papilio
