#ifndef PAPILIO_PAPILIO_HPP
#define PAPILIO_PAPILIO_HPP

#pragma once

#include <tuple>
#include "macros.hpp"
#include "core.hpp"
#include "format.hpp"
#include "print.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT inline constexpr int version_major = PAPILIO_VERSION_MAJOR;
PAPILIO_EXPORT inline constexpr int version_minor = PAPILIO_VERSION_MINOR;
PAPILIO_EXPORT inline constexpr int version_patch = PAPILIO_VERSION_PATCH;

PAPILIO_EXPORT
[[nodiscard]]
constexpr inline std::tuple<int, int, int> get_version() noexcept
{
    return std::make_tuple(
        version_major,
        version_minor,
        version_patch
    );
}
} // namespace papilio

#include "detail/suffix.hpp"

#endif
