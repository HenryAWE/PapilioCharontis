#ifndef PAPILIO_OS_OS_HPP
#define PAPILIO_OS_OS_HPP

#pragma once

#include <papilio/detail/prefix.hpp>
#include <string_view>
#include <system_error>
#include <cstdio>

/**
 * @brief OS-related functions
 */
namespace papilio::os
{
/// @addtogroup OS OS-related functions
/// @{

/**
 * @brief Check if a file is terminal or not
 *
 * @param file File pointer
 */
bool is_terminal(std::FILE* file);

/**
 * @brief Output string to file with conversion from UTF-8 to native encoding
 *
 * This function is as same as @ref output_nonconv on POSIX.
 *
 * @param file File pointer
 * @param out Content **encoded in UTF-8**
 *
 * @throw std::system_error If conversion fails
 */
void output_conv(
    std::FILE* file,
    std::string_view out
);

/**
 * @brief Directly output string to file without any modification
 *
 * @param file File pointer
 * @param out Content
 */
void output_nonconv(
    std::FILE* file,
    std::string_view out
);

/// @}
} // namespace papilio::os

#include <papilio/detail/suffix.hpp>

#endif
