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
#include <papilio_test/setup.hpp>

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

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(push)
#    pragma warning(disable : 4996)
#endif
// The std::tmpfile in the STL shipped with MSVC is marked as deprecated.
// This workaround is clang-cl specific,
// which means it should not be applied to clang on other platforms.
#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

    std::FILE* fp = std::tmpfile();
    if(!fp)
        GTEST_SKIP();

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(pop)
#endif
#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif

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
