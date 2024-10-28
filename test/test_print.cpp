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
    PAPILIO_NS print(fp, "test");
    PAPILIO_NS println(fp);
    fflush(fp);

    char buf[32]{};

    ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
    size_t len = fread(buf, sizeof(char), 10, fp);

    EXPECT_EQ(len, 10);
    EXPECT_EQ(std::string_view(buf, 10), "test\ntest\n");

    fclose(fp);
}

#endif

TEST(print, tmpfile)
{
    using namespace papilio;

    std::FILE* fp = std::tmpfile();
    if(!fp)
        GTEST_SKIP();

    PAPILIO_NS println(fp, "test");
    PAPILIO_NS print(fp, "test");
    PAPILIO_NS println(fp);
    fflush(fp);

    char buf[32]{};

    ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
    size_t len = fread(buf, sizeof(char), 10, fp);

    EXPECT_EQ(len, 10);
    EXPECT_EQ(std::string_view(buf, 10), "test\ntest\n");
}

TEST(print, file_stdout)
{
    using namespace papilio;

    ::testing::internal::CaptureStdout();

    std::string_view fmt = "{} warning{${0}>1:'s'}";

    PAPILIO_NS println(fmt, 1);
    PAPILIO_NS print(fmt, 2);
    PAPILIO_NS println();

    auto stdout_result = ::testing::internal::GetCapturedStdout();
    EXPECT_EQ(
        stdout_result,
        "1 warning\n"
        "2 warnings\n"
    );
}

TEST(print, file_stderr)
{
    using namespace papilio;

    ::testing::internal::CaptureStderr();

    std::string_view fmt = "{} warning{${0}>1:'s'}";

    PAPILIO_NS println(stderr, fmt, 1);
    PAPILIO_NS print(stderr, fmt, 2);
    PAPILIO_NS println(stderr);

    auto stderr_result = ::testing::internal::GetCapturedStderr();
    EXPECT_EQ(
        stderr_result,
        "1 warning\n"
        "2 warnings\n"
    );
}

TEST(print, stream)
{
    using namespace papilio;

    std::ostringstream os;

    PAPILIO_NS println(os, "stream:");
    PAPILIO_NS print(os, "val={val}", "val"_a = 1);
    PAPILIO_NS println(os);
    EXPECT_EQ(os.str(), "stream:\nval=1\n");
}

TEST(print, styled)
{
    using namespace papilio;

    auto styled_helper = []<typename... Args>(bool newline, text_style st, format_string<Args...> fmt, Args&&... args)
    {
        ::testing::internal::CaptureStdout();

        if(newline)
        {
            PAPILIO_NS println(st, fmt, std::forward<Args>(args)...);
        }
        else
        {
            PAPILIO_NS print(st, fmt, std::forward<Args>(args)...);
        }

        auto stdout_result = ::testing::internal::GetCapturedStdout();
        return stdout_result;
    };

    {
        EXPECT_EQ(
            styled_helper(false, style::bold, "hello"),
            "\x1B[1mhello\x1B[0m"
        );
        EXPECT_EQ(
            styled_helper(true, style::bold, "hello"),
            "\x1B[1mhello\x1B[0m\n"
        );
    }

    {
        EXPECT_EQ(
            styled_helper(false, fg(color::yellow) | bg(color::white), "WARNING"),
            "\x1B[33;47mWARNING\x1B[0m"
        );
        EXPECT_EQ(
            styled_helper(true, fg(color::yellow) | bg(color::white), "WARNING"),
            "\x1B[33;47mWARNING\x1B[0m\n"
        );
    }

    {
        EXPECT_EQ(
            styled_helper(false, fg(color::yellow) | bg(color::white) | style::bold, "WARNING"),
            "\x1B[1m\x1B[33;47mWARNING\x1B[0m"
        );
        EXPECT_EQ(
            styled_helper(true, fg(color::yellow) | bg(color::white) | style::bold, "WARNING"),
            "\x1B[1m\x1B[33;47mWARNING\x1B[0m\n"
        );
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
