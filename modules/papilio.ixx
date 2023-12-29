module;
#define PAPILIO_BUILD_MODULES 1
// clang-format off
#include <papilio/macros.hpp>
#include <locale>
#include <iterator>
#include <string>
#include <string_view>
#include <iostream>
// clang-format on

export module papilio;

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(disable : 5244)
#endif

#include <papilio/macros.hpp>
#include <papilio/core.hpp>
#include <papilio/format.hpp>
#include <papilio/papilio.hpp>
