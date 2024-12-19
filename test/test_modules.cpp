#include <gtest/gtest.h>
import papilio;

TEST(modules, version_info)
{
    auto ver = papilio::get_version();
    EXPECT_EQ(std::get<0>(ver), papilio::version_major);
    EXPECT_EQ(std::get<1>(ver), papilio::version_minor);
    EXPECT_EQ(std::get<2>(ver), papilio::version_patch);
}

TEST(modules, format)
{
    EXPECT_EQ(papilio::formatted_size(""), 0);

    EXPECT_EQ(papilio::format("{}", 182376), "182376");
    EXPECT_EQ(papilio::format(L"{}", 182376), L"182376");
}

TEST(modules, print)
{
    ::testing::internal::CaptureStdout();

    std::string_view fmt = "{} warning{${0}>1?'s'}";

    papilio::println(fmt, 1);
    papilio::print(fmt, 2);

    auto stdout_result = ::testing::internal::GetCapturedStdout();
    EXPECT_EQ(
        stdout_result,
        "1 warning\n"
        "2 warnings"
    );
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
