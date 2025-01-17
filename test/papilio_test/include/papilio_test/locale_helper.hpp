#ifndef PAPILIO_TEST_LOCALE_HELPER_HPP
#define PAPILIO_TEST_LOCALE_HELPER_HPP

#pragma once

#include <locale>
#include "prefix.hpp"

namespace papilio_test
{
template <typename CharT>
class yes_no_numpunct : public std::numpunct<CharT>
{
    using my_base = std::numpunct<CharT>;

public:
    using string_type = typename my_base::string_type;

    static constexpr CharT yes_string[] = {'y', 'e', 's', '\0'};

    static constexpr CharT no_string[] = {'n', 'o', '\0'};

protected:
    string_type do_truename() const override
    {
        return yes_string;
    }

    string_type do_falsename() const override
    {
        return no_string;
    }
};

template <typename CharT = char>
std::locale attach_yes_no(const std::locale& loc = std::locale::classic())
{
    return std::locale(loc, new yes_no_numpunct<CharT>());
}
} // namespace papilio_test

#include "suffix.hpp"

#endif
