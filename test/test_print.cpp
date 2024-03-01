#include <gtest/gtest.h>
#include <sstream>
#include <cstdio>
#include <papilio/print.hpp>
#if defined PAPILIO_PLATFORM_LINUX
#    ifndef _GNU_SOURCE
#        define _GNU_SOURCE
#    endif
#    include <sys/mman.h>
#endif

#if defined PAPILIO_PLATFORM_LINUX

TEST(print, file_descriptor_linux)
{
    int fd = memfd_create("test_print", MFD_CLOEXEC);
    if(fd == -1) // Workaround for WSL 1
        GTEST_SKIP() << "memfd_create() failed";
    FILE* fp = fdopen(fd, "wb+");
    ASSERT_TRUE(fp);

    using namespace papilio;

    PAPILIO_NS println(fp, "test");
    fflush(fp);

    char buf[32]{};

    ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
    size_t len = fread(buf, sizeof(char), 5, fp);

    EXPECT_EQ(len, 5);
    EXPECT_STREQ(buf, "test\n");

    fclose(fp);
}

#endif

TEST(print, tmpfile)
{
    using namespace papilio;

    std::FILE* fp = std::tmpfile();

    PAPILIO_NS println(fp, "test");
    PAPILIO_NS print(fp, "test");
    fflush(fp);

    char buf[32]{};

    ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
    size_t len = fread(buf, sizeof(char), 5, fp);

    EXPECT_EQ(len, 5);
    EXPECT_STREQ(buf, "test\n");
}

TEST(print, file_stdout)
{
    using namespace papilio;

    ::testing::internal::CaptureStdout();

    std::string_view fmt = "{} warning{${0}>1:'s'}";

    PAPILIO_NS println(fmt, 1);
    PAPILIO_NS print(fmt, 2);

    auto stdout_result = ::testing::internal::GetCapturedStdout();
    EXPECT_EQ(
        stdout_result,
        "1 warning\n"
        "2 warnings"
    );
}

TEST(print, stream)
{
    using namespace papilio;

    std::ostringstream os;

    PAPILIO_NS println(os, "val={val}", "val"_a = 1);
    EXPECT_EQ(os.str(), "val=1\n");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
