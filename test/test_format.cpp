#include <gtest/gtest.h>
#include <papilio/format.hpp>


static_assert(!papilio::formattable<std::monostate>);
static_assert(!papilio::formattable<bool>);

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

TEST(format, script)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{$ {val}: 'true'}", "val"_a = 1), "true");
}

TEST(format, composite)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{} {}", 182375, 182376), "182375 182376");

    EXPECT_EQ(PAPILIO_NS format("{0} warning{${0}>1:'s'}", 1), "1 warning");
    EXPECT_EQ(PAPILIO_NS format("{0} warning{${0}>1:'s'}", 2), "2 warnings");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
