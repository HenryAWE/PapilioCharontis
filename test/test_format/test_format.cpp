#include <gtest/gtest.h>
#include "test_format.hpp"
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
    return RUN_ALL_TESTS();
}
