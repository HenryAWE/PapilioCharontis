// For only C++

#ifndef PAPILIO_DETAIL_CONFIG_HPP
#define PAPILIO_DETAIL_CONFIG_HPP

#pragma once

#include "config.h"

#if defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
#    define PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT __cpp_multidimensional_subscript
#endif

#endif
