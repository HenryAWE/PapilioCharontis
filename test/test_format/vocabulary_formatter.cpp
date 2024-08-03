#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

TEST(vocabulary_formatter, optional)
{
    using namespace papilio;

    {
        std::optional<std::string> opt = "hello";

        EXPECT_EQ(PAPILIO_NS format("{}", opt), "hello");

        opt.reset();
        EXPECT_EQ(PAPILIO_NS format("{}", opt), "nullopt");
    }

    {
        std::optional<std::wstring> opt = L"hello";

        EXPECT_EQ(PAPILIO_NS format(L"{}", opt), L"hello");

        opt.reset();
        EXPECT_EQ(PAPILIO_NS format(L"{}", opt), L"nullopt");
    }
}

TEST(vocabulary_formatter, variant)
{
    using namespace papilio;
    {
        std::variant<std::monostate, int, std::string> var;
        EXPECT_EQ(PAPILIO_NS format("{}", var), "monostate");

        var.emplace<int>(42);
        EXPECT_EQ(PAPILIO_NS format("{}", var), "42");

        var.emplace<std::string>("hello");
        EXPECT_EQ(PAPILIO_NS format("{}", var), "hello");
    }

    {
        std::variant<std::monostate, int, std::wstring> var;
        EXPECT_EQ(PAPILIO_NS format(L"{}", var), L"monostate");

        var.emplace<int>(42);
        EXPECT_EQ(PAPILIO_NS format(L"{}", var), L"42");

        var.emplace<std::wstring>(L"hello");
        EXPECT_EQ(PAPILIO_NS format(L"{}", var), L"hello");
    }
}
