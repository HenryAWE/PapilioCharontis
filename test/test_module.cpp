#include <gtest/gtest.h>
#include <sstream>
import papilio;
import papilio.script;


TEST(TestModule, MainModule)
{
    EXPECT_NE(papilio::library_info(), nullptr);

    {
        std::stringstream ss;
        papilio::format_to(ss, "{}", 10);
        EXPECT_EQ(ss.str(), "10");
    }
}
TEST(TestModule, ScriptModule)
{
    using namespace papilio;

    script::variable var = 2;
    EXPECT_TRUE(var.holds<script::variable::int_type>());
    EXPECT_EQ(var.get<script::variable::int_type>(), 2);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
