#include <gtest/gtest.h>
#include <sstream>
#include <papilio/print.hpp>
#ifdef PAPILIO_PLATFORM_LINUX
#   ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#   endif
#   include <sys/mman.h>
#endif

#ifdef PAPILIO_PLATFORM_LINUX

TEST(print, file_descriptor_linux)
{
    int fd = memfd_create("test_print", MFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    FILE* fp = fdopen(fd, "wb+");
    ASSERT_TRUE(fp);

    using namespace papilio;

    PAPILIO_NS println(fp);
    fflush(fp);

    char buf[32]{};

    ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
    size_t len = fread(buf, 1, sizeof(buf), fp);

    EXPECT_EQ(len, 1);
    EXPECT_STREQ(buf, "\n");

    fclose(fp);
}

#endif

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
