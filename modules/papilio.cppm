module;
#define PAPILIO_BUILD_MODULES 1
#include <papilio/macros.hpp>
#include <locale>
#include <iterator>
#include <string>
#include <string_view>
#include <iostream>

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
