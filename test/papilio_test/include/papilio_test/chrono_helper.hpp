#ifndef PAPILIO_TEST_CHRONO_HELPER_HPP
#define PAPILIO_TEST_CHRONO_HELPER_HPP

#pragma once

#include <ctime>
#include "prefix.hpp"

namespace papilio_test
{
/**
 * @brief The Unix epoch (January 1, 1970)
 */
inline std::tm create_tm_epoch()
{
    const std::time_t t = 0;
    std::tm result{};

#ifdef PAPILIO_STDLIB_MSVC_STL
    gmtime_s(&result, &t);
#else
    result = *std::gmtime(&t);
#endif

    return result;
}
} // namespace papilio_test

#include "suffix.hpp"

#endif
