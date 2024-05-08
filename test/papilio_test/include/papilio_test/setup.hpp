/**
 * @file setup.hpp
 * @brief Setup compiler options, such as disabling unnecessary warnings.
 */

#ifndef PAPILIO_TEST_SETUP_HPP
#define PAPILIO_TEST_SETUP_HPP

#pragma once

#include <papilio/macros.hpp>

#ifdef PAPILIO_COMPILER_CLANG

#    pragma clang diagnostic ignored "-Wc++98-compat"
#    pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#    pragma clang diagnostic ignored "-Wc++98-c++11-compat-binary-literal"
#    pragma clang diagnostic ignored "-Wpre-c++17-compat"
#    pragma clang diagnostic ignored "-Wpre-c++20-compat-pedantic"
#    pragma clang diagnostic ignored "-Wc++20-compat"

#    pragma clang diagnostic ignored "-Wglobal-constructors"
#    pragma clang diagnostic ignored "-Wcovered-switch-default"
#    pragma clang diagnostic ignored "-Wexit-time-destructors"
#    pragma clang diagnostic ignored "-Wsign-conversion"

#    if __clang_major__ >= 16
#        pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#    endif

#endif

#endif
