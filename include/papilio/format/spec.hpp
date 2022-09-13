#pragma once


namespace papilio::format
{
    enum class sign_format_spec
    {
        positive,
        negative,
        space
    };

    struct integer_format_spec
    {
        int base = 10;
        sign_format_spec sign = sign_format_spec::negative;
    };
    struct floating_format_spec
    {
        sign_format_spec sign = sign_format_spec::negative;
        std::size_t precision = 6;
    };
}
