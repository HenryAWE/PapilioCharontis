#include <gtest/gtest.h>
#include <papilio/container.hpp>

namespace test_container
{
// Static checks
static_assert(papilio::is_transparent_v<std::less<>>);
static_assert(!papilio::is_transparent_v<std::less<std::string>>);
} // namespace test_container

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
