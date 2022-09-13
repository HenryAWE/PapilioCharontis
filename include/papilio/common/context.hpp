#pragma once

#include <variant>
#include <string>
#include "type.hpp"


namespace papilio::common
{
    namespace detailed
    {
        template <typename CharT>
        using varying_arg = std::variant<
            int,
            unsigned int,
            long,
            unsigned long,
            long long,
            unsigned long long,
            bool,
            char,
            float,
            long double,
            std::basic_string_view<CharT>,
            void*
        >;
    }

    template <typename CharT>
    class dynamic_format_arg
    {
    public:
        typedef CharT char_type;



    private:
        data_type m_type;
        detailed::varying_arg<char_type> m_val;
    };
}
