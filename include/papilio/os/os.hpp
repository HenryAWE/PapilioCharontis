#ifndef PAPILIO_OS_OS_HPP
#define PAPILIO_OS_OS_HPP

#pragma once

#include <papilio/detail/prefix.hpp>
#include <string_view>
#include <system_error>
#include <cstdio>

namespace papilio::os
{
bool is_terminal(std::FILE* file);

void output_conv(
    std::FILE* file,
    std::string_view out
);

void output_nonconv(
    std::FILE* file,
    std::string_view out
);
} // namespace papilio::os

#include <papilio/detail/suffix.hpp>

#endif
