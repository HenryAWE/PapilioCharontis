#include <gtest/gtest.h>
#include "test_format.hpp"
#include <papilio/print.hpp>
#include <papilio/chrono/chrono_utility.hpp>
#include <papilio_test/setup.hpp>

namespace test_format
{
std::ostream& operator<<(std::ostream& os, const stream_only&)
{
    os << "stream only";
    return os;
}

std::wostream& operator<<(std::wostream& os, const stream_only&)
{
    os << L"stream only";
    return os;
}
} // namespace test_format

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

#ifdef PAPILIO_HAS_LIB_STACKTRACE
    papilio::println(std::cerr, "PAPILIO_HAS_LIB_STACKTRACE = {:d}L", PAPILIO_HAS_LIB_STACKTRACE);
#endif
#ifdef PAPILIO_HAS_LIB_EXPECTED
    papilio::println(std::cerr, "PAPILIO_HAS_LIB_EXPECTED = {:d}L", PAPILIO_HAS_LIB_EXPECTED);
#endif

#ifdef PAPILIO_CHRONO_NO_UTC_TIME
    papilio::println(std::cerr, "PAPILIO_CHRONO_NO_UTC_TIME defined");
#endif

#ifdef PAPILIO_CHRONO_NO_TIMEZONE
    papilio::println(std::cerr, "PAPILIO_CHRONO_NO_TIMEZONE defined");
#endif

    return RUN_ALL_TESTS();
}
