#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

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
        std::pair<int, int> p(1, 2);

        EXPECT_EQ(PAPILIO_NS format("{:*<10}", p), "(1, 2)****");
        EXPECT_EQ(PAPILIO_NS format("{:*^10}", p), "**(1, 2)**");
        EXPECT_EQ(PAPILIO_NS format("{:*>10}", p), "****(1, 2)");

        EXPECT_EQ(PAPILIO_NS format(L"{:*<10}", p), L"(1, 2)****");
        EXPECT_EQ(PAPILIO_NS format(L"{:*^10}", p), L"**(1, 2)**");
        EXPECT_EQ(PAPILIO_NS format(L"{:*>10}", p), L"****(1, 2)");
    }

    {
        std::pair<int, std::string> kv{1, "value"};

        EXPECT_EQ(PAPILIO_NS format("{:m}", kv), R"(1: "value")");
        EXPECT_EQ(PAPILIO_NS format("{:*^14m}", kv), R"(**1: "value"**)");

        std::pair<int, std::wstring> wkv{1, L"value"};

        EXPECT_EQ(PAPILIO_NS format(L"{:m}", wkv), LR"(1: "value")");
        EXPECT_EQ(PAPILIO_NS format(L"{:*^14m}", wkv), LR"(**1: "value"**)");
    }
}
