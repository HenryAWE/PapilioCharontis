#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(vocabulary_formatter, optional)
{
    using namespace papilio;

    static_assert(formattable<std::optional<std::string>>);
    static_assert(!formattable<std::optional<test_format::format_disabled>>);

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

    static_assert(formattable<std::variant<int, std::string>>);
    static_assert(formattable<std::variant<std::monostate, std::string>>);
    static_assert(!formattable<std::variant<test_format::format_disabled>>);
    static_assert(!formattable<std::variant<int, test_format::format_disabled>>);

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

#ifdef PAPILIO_HAS_LIB_EXPECTED

TEST(vocabulary_formatter, expected)
{
    using namespace papilio;

    static_assert(formattable<std::expected<std::string, int>>);
    static_assert(!formattable<std::expected<test_format::format_disabled, int>>);
    static_assert(!formattable<std::expected<std::string, test_format::format_disabled>>);

    {
        std::expected<std::string, int> ex = "hello";

        EXPECT_EQ(PAPILIO_NS format("{}", ex), "hello");
    }
    {
        std::expected<std::string, int> ex = std::unexpected(42);

        EXPECT_EQ(PAPILIO_NS format("{}", ex), "42");
    }

    {
        std::expected<std::wstring, int> ex = L"hello";

        EXPECT_EQ(PAPILIO_NS format(L"{}", ex), L"hello");
    }
    {
        std::expected<std::wstring, int> ex = std::unexpected(42);

        EXPECT_EQ(PAPILIO_NS format(L"{}", ex), L"42");
    }
}

#endif
