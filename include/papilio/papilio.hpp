#pragma once

#include <tuple>
#include "core.hpp"
#include "script.hpp"
#include "format.hpp"
#include "print.hpp"


namespace papilio
{
    inline constexpr int version_major = 0;
    inline constexpr int version_minor = 1;
    inline constexpr int version_patch = 0;

    [[nodiscard]]
    constexpr inline std::tuple<int, int, int> get_version() noexcept
    {
        return std::make_tuple(
            version_major,
            version_minor,
            version_patch
        );
    }
}
