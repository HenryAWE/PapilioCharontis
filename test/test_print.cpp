#include <gtest/gtest.h>
#include <sstream>
#include <papilio/papilio.hpp>


TEST(TestPrint, PrintToStream)
{
    using namespace papilio;

    {
        std::stringstream ss;
        print(ss, "{}", "hello");

        EXPECT_EQ(ss.str(), "hello");
    }

    {
        std::stringstream ss;
        print(ss, "{}", 1234);

        EXPECT_EQ(ss.str(), "1234");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
