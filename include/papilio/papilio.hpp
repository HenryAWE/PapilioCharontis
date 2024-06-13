/**
 * @file papilio.hpp
 * @author HenryAWE
 * @brief Main header file of the library
 *
 * @note Although this header will automatically include most of the library,
 *       some format support for seldom used types still need to be included separately.
 */

#ifndef PAPILIO_PAPILIO_HPP
#define PAPILIO_PAPILIO_HPP

#pragma once

#include <tuple>
#include "macros.hpp"
#include "core.hpp"
#include "format.hpp"
#include "print.hpp"
#include "detail/prefix.hpp"

/**
 * @brief The main namespace of Papilio Charontis
 */
namespace papilio
{
PAPILIO_EXPORT inline constexpr int version_major = PAPILIO_VERSION_MAJOR;
PAPILIO_EXPORT inline constexpr int version_minor = PAPILIO_VERSION_MINOR;
PAPILIO_EXPORT inline constexpr int version_patch = PAPILIO_VERSION_PATCH;

/**
 * @brief Get the version number
 *
 * @return constexpr std::tuple<int, int, int> The version number
 */
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
