#pragma once

#include <cstdint>
#include <type_traits>
#include <string>
#include <variant>
#include <any>


namespace papilio::common
{
    enum class data_type
    {
        none_type = -1,
        // Integers
        int_type,
        uint_type,
        long_type,
        ulong_type,
        long_long_type,
        ulong_long_type,
        bool_type,
        char_type,
        last_int_type = char_type,
        // Floating points
        float_type,
        double_type,
        long_double_type,
        last_numeric_type = long_double_type,
        cstring_type,
        string_type,
        pointer_type,
        custom_type,
        last_data_type = custom_type
    };

    namespace detailed
    {
        template <typename T>
        consteval bool is_char() noexcept
        {
            using namespace std;;
            using U = remove_cv_t<T>;

            return
                is_same_v<U, char> ||
                is_same_v<U, wchar_t> ||
                is_same_v<U, char16_t> ||
                is_same_v<U, char32_t> ||
                is_same_v<U, char8_t>;
        }

        template <typename T>
        inline constexpr bool is_char_v = is_char<T>();

        template <typename T>
        struct is_string : public std::false_type {};
        template <typename CharT>
        struct is_string<std::basic_string<CharT>> : public std::true_type {};

        template <typename T>
        inline constexpr bool is_string_v = is_string<T>::value;

        constexpr auto to_underlying(data_type t) noexcept
        {
            return static_cast<std::underlying_type_t<data_type>>(t);
        }
    }

    template <data_type Type>
    struct referred_type
    {
        using type = void;
    };
    template <>
    struct referred_type<data_type::int_type>
    {
        using type = int;
    };

    template <typename T>
    consteval data_type get_data_type() noexcept
    {
        using namespace std;

        if constexpr(is_same_v<T, int> || is_same_v<T, short> || is_same_v<T, signed char>)
            return data_type::int_type;
        else if constexpr(is_same_v<T, unsigned int> || is_same_v<T, unsigned short> || is_same_v<T, unsigned char>)
            return data_type::uint_type;
        else if constexpr(is_same_v<T, long>)
            return data_type::long_type;
        else if constexpr(is_same_v<T, unsigned long>)
            return data_type::ulong_type;
        else if constexpr(is_same_v<T, long long>)
            return data_type::long_long_type;
        else if constexpr(is_same_v<T, unsigned long long>)
            return data_type::ulong_long_type;
        else if constexpr(is_same_v<T, bool>)
            return data_type::bool_type;
        else if constexpr(detailed::is_char<T>())
            return data_type::char_type;
        else if constexpr(is_same_v<T, float>)
            return data_type::float_type;
        else if constexpr(is_same_v<T, double>)
            return data_type::double_type;
        else if constexpr(is_same_v<T, long double>)
            return data_type::long_double_type;
        else if constexpr(is_same_v<T, long double>)
            return data_type::long_double_type;
        else if constexpr(is_pointer_v<T> || is_null_pointer_v<T>)
        {
            if constexpr(detailed::is_char_v<remove_pointer_t<T>>)
                return data_type::cstring_type;
            else
                return data_type::pointer_type;
        }
        else if constexpr(detailed::is_string_v<T>)
            return data_type::string_type;
        else
        {
            if constexpr(is_void_v<T>)
                return data_type::none_type;
            else
                return data_type::custom_type;
        }
    }

    constexpr bool is_integer(data_type t) noexcept
    {
        return
            detailed::to_underlying(t) >= detailed::to_underlying(data_type::int_type) &&
            detailed::to_underlying(t) <= detailed::to_underlying(data_type::last_int_type);
    }
    constexpr bool is_floating_point(data_type t) noexcept
    {
        return
            detailed::to_underlying(t) >= detailed::to_underlying(data_type::float_type) &&
            detailed::to_underlying(t) <= detailed::to_underlying(data_type::last_numeric_type);
    }
    constexpr bool is_numeric(data_type t) noexcept
    {
        return
            detailed::to_underlying(t) >= detailed::to_underlying(data_type::int_type) &&
            detailed::to_underlying(t) <= detailed::to_underlying(data_type::last_numeric_type);
    }
    constexpr data_type to_unsigned(data_type t) noexcept
    {
        using detailed::to_underlying, std::make_unsigned_t;

        switch(t)
        {
        case data_type::int_type:
        case data_type::long_type:
        case data_type::long_long_type:
            return static_cast<data_type>(to_underlying(t) + 1);

        case data_type::uint_type:
        case data_type::ulong_type:
        case data_type::ulong_long_type:
            return t;

        case data_type::char_type:
            return data_type::uint_type;

        case data_type::pointer_type:
            return get_data_type<make_unsigned_t<std::uintptr_t>>();

        case data_type::bool_type:
        case data_type::float_type:
        case data_type::double_type:
        case data_type::long_double_type:
        default:
            return data_type::none_type;
        };
    }

    data_type common_data_type(data_type t, data_type u)
    {
        using std::max, detailed::to_underlying;

        if(t == u)
            return t;
        else if(t == data_type::none_type || u == data_type::none_type)
            return data_type::none_type;
        else if(is_floating_point(t) && is_numeric(u) || is_numeric(t) && is_floating_point(u))
        {
            return static_cast<data_type>(max(to_underlying(t), to_underlying(u)));
        }
        else if(is_integer(t) && is_integer(u))
        {
            
        }
        else if(t == data_type::cstring_type && u == data_type::string_type)
            return data_type::string_type;

        return data_type::none_type;
    }
}
