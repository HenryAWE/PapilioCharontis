#include <gtest/gtest.h>
#include <sstream>
#include <papilio/print.hpp>


TEST(print, stream)
{
    using namespace papilio;

    std::ostringstream os;

    PAPILIO_NS println(os);
    EXPECT_EQ(os.str(), "\n");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
