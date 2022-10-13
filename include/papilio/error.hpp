#pragma once

#include <stdexcept>


namespace papilio
{
    class invalid_format : public std::invalid_argument
    {
    public:
        using invalid_argument::invalid_argument;
    };
    class invalid_operation : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };
}
