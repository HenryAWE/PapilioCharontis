#include <gtest/gtest.h>
import papilio;

TEST(modules, version_info)
{
    auto ver = papilio::get_version();
    EXPECT_EQ(std::get<0>(ver), papilio::version_major);
    EXPECT_EQ(std::get<1>(ver), papilio::version_minor);
    EXPECT_EQ(std::get<2>(ver), papilio::version_patch);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
