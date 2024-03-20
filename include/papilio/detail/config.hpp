// For only C++

#ifndef PAPILIO_DETAIL_CONFIG_HPP
#define PAPILIO_DETAIL_CONFIG_HPP

#pragma once

#include "config.h"

#if defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
#    define PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT __cpp_multidimensional_subscript
#endif

#if defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
#    define PAPILIO_HAS_STACKTRACE __cpp_lib_stacktrace
#endif

#endif
