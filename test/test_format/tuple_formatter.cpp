#include <gtest/gtest.h>
#include <papilio/format.hpp>

TEST(tuple_formatter, basic)
{
    using namespace papilio;

    {
        std::tuple<> empty_tp;

        EXPECT_EQ(PAPILIO_NS format("{}", empty_tp), "()");
        EXPECT_EQ(PAPILIO_NS format("{:n}", empty_tp), "");

        EXPECT_EQ(PAPILIO_NS format(L"{}", empty_tp), L"()");
        EXPECT_EQ(PAPILIO_NS format(L"{:n}", empty_tp), L"");
    }

    {
        std::tuple<int, int, int> tp{1, 2, 3};

        EXPECT_EQ(PAPILIO_NS format("{}", tp), "(1, 2, 3)");
        EXPECT_EQ(PAPILIO_NS format("{:n}", tp), "1, 2, 3");

        EXPECT_EQ(PAPILIO_NS format(L"{}", tp), L"(1, 2, 3)");
        EXPECT_EQ(PAPILIO_NS format(L"{:n}", tp), L"1, 2, 3");
    }

    {
        std::pair<int, std::string> kv{1, "value"};

        EXPECT_EQ(PAPILIO_NS format("{:m}", kv), "1: value");

        std::pair<int, std::wstring> wkv{1, L"value"};

        EXPECT_EQ(PAPILIO_NS format(L"{:m}", wkv), L"1: value");
    }
}
