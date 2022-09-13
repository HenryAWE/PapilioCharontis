#pragma once

#include <cassert>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <string>
#include "spec.hpp"
#include "../common/type.hpp"


namespace papilio::format
{
    namespace detailed
    {
        template <typename T>
        constexpr auto ipow(int base, T exp) noexcept
            ->std::enable_if_t<std::is_integral_v<T>, std::common_type_t<int, T>>
        {
            using U = std::common_type_t<int, T>;
            assert(exp >= 0);
            if(exp == 0)
                return 1;
            return static_cast<U>(base) * ipow(base, exp - 1);
        }

        template <typename T>
        auto count_digit(T val, int base)
            ->std::enable_if_t<std::is_integral_v<T>, std::size_t>
        {
            if(val == 0)
                return 1;
            std::size_t n = 0;
            while(val != 0)
            {
                val /= base;
                ++n;
            }
            return n;
        }

        template <typename OutputIt, typename CharT>
        OutputIt output_sequence(OutputIt out, std::basic_string_view<CharT> view)
        {
            return std::copy(view.begin(), view.end(), out);
        }
        template <typename OutputIt, typename CharT>
        OutputIt output_sequence(OutputIt out, const CharT* cstr)
        {
            return output_sequence(out, std::basic_string_view<CharT>(cstr));
        }

        template <typename OutputIt, typename T>
        auto output_sign(OutputIt it, T val, sign_format_spec spec)->
            std::enable_if_t<std::is_integral_v<T>, OutputIt>
        {
            if(spec == sign_format_spec::positive)
            {
                *it = val < 0 ? '-' : '+';
                ++it;
            }
            else if(spec == sign_format_spec::space)
            {
                *it = val < 0 ? '-' : ' ';
                ++it;
            }
            else if(spec == sign_format_spec::negative && val < 0)
            {
                *it = '-';
                ++it;
            }

            return it;
        }
        // WARNING: This function doesn't handle NaN
        template <typename OutputIt, typename T>
        auto output_sign(OutputIt it, T val, sign_format_spec spec)->
            std::enable_if_t<std::is_floating_point_v<T>, OutputIt>
        {
            if(spec == sign_format_spec::positive)
            {
                *it = std::signbit(val) ? '-' : '+';
                ++it;
            }
            else if(spec == sign_format_spec::space)
            {
                *it = std::signbit(val) ? '-' : ' ';
                ++it;
            }
            else if(spec == sign_format_spec::negative && std::signbit(val))
            {
                *it = '-';
                ++it;
            }

            return it;
        }
    }

    template <typename OutputIt, typename T>
    auto vformat_to(OutputIt it, T val, integer_format_spec spec = integer_format_spec{})->
        std::enable_if_t<std::is_integral_v<T>, OutputIt>
    {
        it = detailed::output_sign(it, val, spec.sign);

        if(val == 0)
        {
            *it = '0';
            return it;
        }
        std::size_t n = detailed::count_digit(val, spec.base);
        constexpr char table[]
        {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'a', 'b', 'c', 'd', 'e', 'f'
        };
        assert(spec.base >= 2 && spec.base <= 16);
        if constexpr(std::is_signed_v<T>)
            val = std::abs(val);
        while(n != 0)
        {
            auto digit = (val % detailed::ipow(spec.base, n)) / detailed::ipow(spec.base, n - 1);
            --n;
            *it = table[digit];
            ++it;
        }
        return it;
    }
    template <typename OutputIt, typename T>
    auto vformat_to(OutputIt it, T val, floating_format_spec spec = floating_format_spec{})->
        std::enable_if_t<std::is_floating_point_v<T>, OutputIt>
    {
        if(std::isnan(val))
        {
            return detailed::output_sequence(it, "nan");
        }
        it = detailed::output_sign(it, val, spec.sign);
        ++it;
        if(std::isinf(val))
        {
            return detailed::output_sequence(it, "inf");
        }

        T ival;
        T fval = std::modf(val, &ival);
        std::size_t int_digit = detailed::count_digit(static_cast<unsigned int>(std::abs(ival)), 10);
        
        if(std::abs(fval) > std::numeric_limits<T>::epsilon())
        {
            *it = '.';
            ++it;
            std::size_t zero_count = 0;
            std::size_t n = spec.precision;
            while(n != 0)
            {
                fval *= static_cast<T>(10);
                T digit;
                fval = std::modf(fval, &digit);
                --n;
                assert(int(digit) <= 9);
                if(std::abs(fval) < std::numeric_limits<T>::epsilon())
                {
                    ++zero_count;
                }
                else
                {
                    if(zero_count)
                    {
                        it = std::fill_n(it, zero_count, '0');
                        zero_count = 0;
                    }
                    *it = '0' + int(digit);
                    ++it;
                }
            }
            if(zero_count == spec.precision)
            { // Output "xxx.0"
                *it = '0';
                ++it;
            }
        }
        return it;
    }

    template <typename T>
    constexpr auto vformatted_size(T val, integer_format_spec spec = integer_format_spec{}) noexcept->
        std::enable_if_t<std::is_integral_v<T>, T>
    {
        std::size_t n = 0;
        if(spec.sign == sign_format_spec::positive || spec.sign == sign_format_spec::space)
            ++n;
        else if(spec.sign == sign_format_spec::negative && val < 0)
            ++n;
        return n + detailed::count_digit(val, spec.base);
    }
    template <typename CharT>
    constexpr auto vformatted_size(std::basic_string_view<CharT> view) noexcept->
        std::enable_if_t<common::detailed::is_char_v<CharT>, std::size_t>
    {
        return view.size();
    }
    template <typename CharT>
    constexpr auto vformatted_size(const CharT* cstr) noexcept->
        std::enable_if_t<common::detailed::is_char_v<CharT>, std::size_t>
    {
        return vformatted_size(std::basic_string_view<CharT>(cstr));
    }
}
