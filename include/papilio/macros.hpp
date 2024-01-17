#pragma once

#ifdef __cplusplus
#    include <cassert>
#    include <version>
#    include <type_traits> // std::is_same_v
#else
#    include <assert.h>
#endif

// clang-format off
// CMakeLists.txt will use following macros to determine library version.

#define PAPILIO_VERSION_MAJOR 0
#define PAPILIO_VERSION_MINOR 1
#define PAPILIO_VERSION_PATCH 0

// clang-format on

#ifdef _MSC_VER
#    define PAPILIO_COMPILER_MSVC _MSC_VER
#endif
#ifdef __GNUC__
#    define PAPILIO_COMPILER_GCC __GNUC__
#endif

#define PAPILIO_ASSERT(expr) assert(expr)

#if defined(_WIN32) || defined(_WIN64)
#    define PAPILIO_PLATFORM_WINDOWS 1
#elif defined(__linux__) || defined(__gnu_linux__)
#    define PAPILIO_PLATFORM_LINUX 1
#endif

#define PAPILIO_HAS_VA_OPT_HELPER(_0, _1, result, ...) result
#define PAPILIO_HAS_VA_OPT_IMPL(...) \
    PAPILIO_HAS_VA_OPT_HELPER(__VA_OPT__(, ), 1, 0)

#if PAPILIO_HAS_VA_OPT_IMPL(_0) == 1
#    define PAPILIO_HAS_VA_OPT 1
#endif

#define PAPILIO_ERR_NO_ERROR           0x0
#define PAPILIO_ERR_END_OF_STRING      0x1
#define PAPILIO_ERR_INVALID_FIELD_NAME 0x2
#define PAPILIO_ERR_INVALID_CONDITION  0x3
#define PAPILIO_ERR_INVALID_INDEX      0x4
#define PAPILIO_ERR_INVALID_ATTRIBUTE  0x5
#define PAPILIO_ERR_INVALID_OPERATOR   0x6
#define PAPILIO_ERR_INVALID_STRING     0x7
#define PAPILIO_ERR_UNCLOSED_BRACE     0x8

#define PAPILIO_ERR_UNKNOWN_ERROR      -1

#ifdef __cplusplus // C compatibility
#    define PAPILIO_NS ::papilio::

#    if defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
#        define PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT __cpp_multidimensional_subscript
#    endif

#    ifdef PAPILIO_COMPILER_MSVC
// [[no_unique_address]] is ignored by MSVC even in C++20 mode.
// https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/#msvc-extensions-and-abi
#        define PAPILIO_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#    else
#        define PAPILIO_NO_UNIQUE_ADDRESS [[no_unique_address]]
#    endif

#    ifdef PAPILIO_BUILD_MODULES
#        define PAPILIO_EXPORT export
#    else
#        define PAPILIO_EXPORT
#    endif

#    if __has_cpp_attribute(assume)
#        define PAPILIO_ASSUME(expr) [[assume(expr)]]
#    elif defined PAPILIO_COMPILER_MSVC
#        define PAPILIO_ASSUME(expr) __assume(expr)
#    elif defined PAPILIO_COMPILER_GCC
#        if PAPILIO_COMPILER_GCC >= 13
#            define PAPILIO_ASSUME(expr) __attribute__((assume(expr)))
#        else
#            define PAPILIO_ASSUME(expr) PAPILIO_ASSERT(expr)
#        endif
#    else
#        define PAPILIO_ASSUME(expr)
#    endif

#    define PAPILIO_STRINGIZE(text)    #text
#    define PAPILIO_STRINGIZE_EX(text) PAPILIO_STRINGIZE(text)

#    define PAPILIO_TSTRING_EX(char_t, str, suffix, decl)         \
        []() noexcept -> decltype(auto)                           \
        {                                                         \
            decl;                                                 \
            if constexpr(::std::is_same_v<char_t, char>)          \
                return str##suffix;                               \
            else if constexpr(::std::is_same_v<char_t, wchar_t>)  \
                return L##str##suffix;                            \
            else if constexpr(::std::is_same_v<char_t, char8_t>)  \
                return u8##str##suffix;                           \
            else if constexpr(::std::is_same_v<char_t, char32_t>) \
                return U##str##suffix;                            \
            else if constexpr(::std::is_same_v<char_t, char16_t>) \
                return u##str##suffix;                            \
        }()

#    define PAPILIO_TSTRING(char_t, str) PAPILIO_TSTRING_EX(char_t, str, , )

// NOTE: #include <string_view> first
#    define PAPILIO_TSTRING_VIEW(char_t, str) PAPILIO_TSTRING_EX( \
        char_t, str, sv, using namespace ::std                    \
    )

// NOTE: #include <papilio/detail/compat.hpp> first
#    define PAPILIO_UNREACHABLE() (PAPILIO_NS detail::unreachable())

#endif
