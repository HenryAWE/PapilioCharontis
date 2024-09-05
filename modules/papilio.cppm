module;
#define PAPILIO_BUILD_MODULES 1
#include <papilio/detail/config.hpp>

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#    pragma clang diagnostic ignored "-Wmissing-prototypes"
#    pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

#include <limits>
#include <locale>
#include <charconv>
#include <memory>
#include <array>
#include <vector>
#include <map>
#include <ranges>
#include <variant>
#include <iterator>
#include <algorithm>
#include <utility>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>
#include <typeindex>
#include <typeinfo>

export module papilio;

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(disable : 5244)
#endif

#include <papilio/macros.hpp>
#include <papilio/core.hpp>
#include <papilio/format.hpp>
#include <papilio/papilio.hpp>

#include "../src/container.cpp"
#include "../src/os/general.cpp"
#include "../src/os/winapi.cpp"
#include "../src/os/posix.cpp"
#include "../src/utf/stralgo.cpp"
#include "../src/utf/codepoint.cpp"
#include "../src/utf/string.cpp"
#include "../src/locale.cpp"
#include "../src/core.cpp"
#include "../src/access.cpp"
#include "../src/format.cpp"
#include "../src/color.cpp"
#include "../src/print.cpp"
