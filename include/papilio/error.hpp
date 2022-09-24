#pragma once

#include <stdexcept>


namespace papilio
{
    class invalid_operation : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };
}
