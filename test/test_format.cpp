#include <gtest/gtest.h>
#include <papilio/format.hpp>


TEST(format, plain_text)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format(""), "");
    EXPECT_EQ(PAPILIO_NS format("plain text"), "plain text");
    EXPECT_EQ(PAPILIO_NS format("{{plain text}}"), "{plain text}");
}

TEST(format, exception)
{
    using namespace papilio;

    EXPECT_THROW(PAPILIO_NS format("{"), papilio::invalid_format);
    EXPECT_THROW(PAPILIO_NS format("}"), papilio::invalid_format);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
