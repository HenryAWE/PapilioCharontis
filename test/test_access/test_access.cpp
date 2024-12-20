#include <gtest/gtest.h>
#include <papilio/access.hpp>
#include <papilio/accessor/misc.hpp>
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

TEST(indexing_value, constructor)
{
    using namespace papilio;

    {
        indexing_value idx(0);
        ASSERT_TRUE(idx.holds_index());
        EXPECT_EQ(idx.as_index(), 0);
    }

    {
        indexing_value idx(-1);
        ASSERT_TRUE(idx.holds_index());
        EXPECT_EQ(idx.as_index(), -1);
        EXPECT_LT(idx.as_index(), 0);
    }

    {
        indexing_value idx(index_range(0, 1));
        ASSERT_TRUE(idx.holds_range());
        EXPECT_EQ(idx.as_range().first, 0);
        EXPECT_EQ(idx.as_range().second, 1);
    }

    {
        indexing_value idx("hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), "hello");
        EXPECT_FALSE(idx.as_string().has_ownership());
    }

    {
        indexing_value idx(independent, "hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), "hello");
        EXPECT_TRUE(idx.as_string().has_ownership());
    }

    {
        windexing_value idx(L"hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), L"hello");
        EXPECT_FALSE(idx.as_string().has_ownership());
    }

    {
        windexing_value idx(independent, L"hello");
        ASSERT_TRUE(idx.holds_string());
        EXPECT_EQ(idx.as_string(), L"hello");
        EXPECT_TRUE(idx.as_string().has_ownership());
    }
}

TEST(attribute_name, compare)
{
    using namespace std::literals;
    using papilio::attribute_name;

    auto attr = attribute_name("name");
    EXPECT_FALSE(attr.name().has_ownership());

    EXPECT_EQ("name", attr);
    EXPECT_EQ(attr, "name");
    EXPECT_EQ("name"s, attr);
    EXPECT_EQ(attr, "name"s);
    EXPECT_EQ("name"sv, attr);
    EXPECT_EQ(attr, "name"sv);

    EXPECT_NE(attr, "{name}");
    EXPECT_NE("{name}", attr);
}

TEST(accessor, string)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{.size}", "hello"), "5");
    EXPECT_EQ(PAPILIO_NS format("{.length}", "hello"), "5");
}

TEST(accessor, tuple)
{
    using namespace papilio;

    {
        std::tuple<> empty;

        EXPECT_EQ(PAPILIO_NS format("{.size}", empty), "0");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", empty), L"0");
    }

    {
        std::pair<std::string, int> val("scene", 182376);

        EXPECT_EQ(PAPILIO_NS format("{.size}", val), "2");
        EXPECT_EQ(PAPILIO_NS format("{0.first} {0.second}", val), "scene 182376");
    }

    {
        std::pair<std::wstring, int> val(L"scene", 182376);

        EXPECT_EQ(PAPILIO_NS format(L"{.size}", val), L"2");
        EXPECT_EQ(PAPILIO_NS format(L"{0.first} {0.second}", val), L"scene 182376");
    }

    {
        compressed_pair<std::string, int> val("scene", 182376);

        EXPECT_EQ(PAPILIO_NS format("{.size}", val), "2");
        EXPECT_EQ(PAPILIO_NS format("{0.first} {0.second}", val), "scene 182376");
    }

    {
        compressed_pair<std::wstring, int> val(L"scene", 182376);

        EXPECT_EQ(PAPILIO_NS format(L"{.size}", val), L"2");
        EXPECT_EQ(PAPILIO_NS format(L"{0.first} {0.second}", val), L"scene 182376");
    }
}

TEST(accessor, contiguous_range)
{
    using namespace papilio;

    {
        int arr[] = {0, 1, 2, 3};
        std::span<const int> view = arr;

        EXPECT_EQ(PAPILIO_NS format("{.size}", view), "4");
        EXPECT_EQ(PAPILIO_NS format("{.size}", arr), "4");
        EXPECT_EQ(PAPILIO_NS format("{[1:4].size}", view), "3");
        EXPECT_EQ(PAPILIO_NS format("{[1:4].size}", arr), "3");
        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]},{0[2]},{0[3]}", view), "0,1,2,3");
        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]},{0[2]},{0[3]}", arr), "0,1,2,3");

        EXPECT_EQ(PAPILIO_NS format(L"{.size}", view), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", arr), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{[1:4].size}", view), L"3");
        EXPECT_EQ(PAPILIO_NS format(L"{[1:4].size}", arr), L"3");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]},{0[2]},{0[3]}", view), L"0,1,2,3");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]},{0[2]},{0[3]}", arr), L"0,1,2,3");
    }

    {
        std::array<int, 4> arr = {0, 1, 2, 3};

        EXPECT_EQ(PAPILIO_NS format("{.size}", arr), "4");
        EXPECT_EQ(PAPILIO_NS format("{.size}", arr), "4");
        EXPECT_EQ(PAPILIO_NS format("{[1:4].size}", arr), "3");
        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]},{0[2]},{0[3]}", arr), "0,1,2,3");

        EXPECT_EQ(PAPILIO_NS format(L"{.size}", arr), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", arr), L"4");
        EXPECT_EQ(PAPILIO_NS format(L"{[1:4].size}", arr), L"3");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]},{0[2]},{0[3]}", arr), L"0,1,2,3");
    }

    {
        std::vector<int> vi = {0, 1};

        EXPECT_EQ(PAPILIO_NS format("{.size}", vi), "2");
        EXPECT_EQ(PAPILIO_NS format(L"{.size}", vi), L"2");

        EXPECT_EQ(PAPILIO_NS format("{0[0]},{0[1]}", vi), "0,1");
        EXPECT_EQ(PAPILIO_NS format(L"{0[0]},{0[1]}", vi), L"0,1");
    }
}

TEST(accessor, map)
{
    using namespace papilio;

    {
        std::map<int, std::string> m{
            {0,  "zero"},
            {2,   "two"},
            {3, "three"}
        };

        EXPECT_EQ(PAPILIO_NS format("{[0]}", m), "zero");
        EXPECT_EQ(PAPILIO_NS format("{[2]}", m), "two");
        EXPECT_EQ(PAPILIO_NS format("{.min}", m), "zero");
        EXPECT_EQ(PAPILIO_NS format("{.max}", m), "three");

        std::map<int, std::string, std::greater<int>> m_desc{
            {0,  "zero"},
            {2,   "two"},
            {3, "three"}
        };

        EXPECT_EQ(PAPILIO_NS format("{[0]}", m_desc), "zero");
        EXPECT_EQ(PAPILIO_NS format("{[2]}", m_desc), "two");
        EXPECT_EQ(PAPILIO_NS format("{.min}", m_desc), "zero");
        EXPECT_EQ(PAPILIO_NS format("{.max}", m_desc), "three");
    }

    {
        std::map<int, std::wstring> m{
            {0,  L"zero"},
            {2,   L"two"},
            {3, L"three"}
        };

        EXPECT_EQ(PAPILIO_NS format(L"{[0]}", m), L"zero");
        EXPECT_EQ(PAPILIO_NS format(L"{[2]}", m), L"two");
        EXPECT_EQ(PAPILIO_NS format(L"{.min}", m), L"zero");
        EXPECT_EQ(PAPILIO_NS format(L"{.max}", m), L"three");

        std::map<int, std::wstring, std::greater<int>> m_desc{
            {0,  L"zero"},
            {2,   L"two"},
            {3, L"three"}
        };

        EXPECT_EQ(PAPILIO_NS format(L"{[0]}", m_desc), L"zero");
        EXPECT_EQ(PAPILIO_NS format(L"{[2]}", m_desc), L"two");
        EXPECT_EQ(PAPILIO_NS format(L"{.min}", m_desc), L"zero");
        EXPECT_EQ(PAPILIO_NS format(L"{.max}", m_desc), L"three");
    }

    {
        std::map<std::string, int> m{
            {"zero", 0},
            { "two", 2}
        };

        EXPECT_EQ(PAPILIO_NS format("{['zero']}", m), "0");
        EXPECT_EQ(PAPILIO_NS format("{['two']}", m), "2");

        std::map<std::string, int, std::less<>> m_transparent{
            {"zero", 0},
            { "two", 2}
        };

        EXPECT_EQ(PAPILIO_NS format("{['zero']}", m_transparent), "0");
        EXPECT_EQ(PAPILIO_NS format("{['two']}", m_transparent), "2");
    }

    {
        std::map<std::wstring, int> m{
            {L"zero", 0},
            { L"two", 2}
        };

        EXPECT_EQ(PAPILIO_NS format(L"{['zero']}", m), L"0");
        EXPECT_EQ(PAPILIO_NS format(L"{['two']}", m), L"2");
    }
}

TEST(accessor, type_info)
{
    using namespace papilio;

    // The actual stored type is std::type_index
    static_assert(attribute_accessible<std::type_index>);
    static_assert(attribute_accessible_with<std::type_index, wformat_context>);

    {
        const std::type_info& info = typeid(int);
        EXPECT_EQ(
            PAPILIO_NS format("{0.name}: {0.hash_code}", typeid(int)),
            PAPILIO_NS format("{}: {}", info.name(), info.hash_code())
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{0.name}: {0.hash_code}", typeid(int)),
            PAPILIO_NS format(L"{}: {}", utf::string_ref(info.name()).to_wstring(), info.hash_code())
        );
    }
}

TEST(accessor, vector_bool)
{
    using namespace papilio;

    std::vector<bool> vec{true, true, false, true};

    EXPECT_EQ(PAPILIO_NS format("{.size}", vec), "4");
    EXPECT_EQ(PAPILIO_NS format(L"{.size}", vec), L"4");

    EXPECT_EQ(PAPILIO_NS format("{0[0]} {0[-2]}", vec), "true false");
    EXPECT_EQ(PAPILIO_NS format(L"{0[0]} {0[-2]}", vec), L"true false");
}

TEST(accessor, optional)
{
    using namespace papilio;

    {
        std::optional<int> empty = std::nullopt;

        EXPECT_EQ(PAPILIO_NS format("{.has_value}", empty), "false");
        EXPECT_EQ(PAPILIO_NS format(L"{.has_value}", empty), L"false");

        EXPECT_THROW(
            (void)PAPILIO_NS format("{.value}", empty),
            format_error
        );
        EXPECT_THROW(
            (void)PAPILIO_NS format(L"{.value}", empty),
            format_error
        );

        EXPECT_EQ(
            PAPILIO_NS format("{$ !{0.has_value} ? 'empty'}", empty),
            "empty"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{$ !{0.has_value} ? 'empty'}", empty),
            L"empty"
        );
    }

    {
        std::optional<int> val = 42;

        EXPECT_EQ(PAPILIO_NS format("{.has_value}", val), "true");
        EXPECT_EQ(PAPILIO_NS format(L"{.has_value}", val), L"true");

        EXPECT_EQ(PAPILIO_NS format("{$ {.has_value} ? 'non-empty'}", val), "non-empty");
        EXPECT_EQ(PAPILIO_NS format(L"{$ {.has_value} ? 'non-empty'}", val), L"non-empty");

        EXPECT_EQ(PAPILIO_NS format("{.value}", val), "42");
        EXPECT_EQ(PAPILIO_NS format(L"{.value}", val), L"42");

        EXPECT_EQ(PAPILIO_NS format("{.value:*>4}", val), "**42");
        EXPECT_EQ(PAPILIO_NS format(L"{.value:*>4}", val), L"**42");
    }
}

TEST(accessor, variant)
{
    using namespace papilio;

    {
        std::variant<int, float> var = 42;

        EXPECT_EQ(PAPILIO_NS format("{.index}", var), "0");
        EXPECT_EQ(PAPILIO_NS format("{.value:*>4}", var), "**42");
        EXPECT_EQ(PAPILIO_NS format("{[0]}", var), "42");
        EXPECT_THROW((void)PAPILIO_NS format("{[1]}", var), format_error);
        EXPECT_EQ(PAPILIO_NS format("{[-2]}", var), "42");
        EXPECT_THROW((void)PAPILIO_NS format("{[2]}", var), format_error);
        EXPECT_THROW((void)PAPILIO_NS format("{[-3]}", var), format_error);

        EXPECT_EQ(PAPILIO_NS format(L"{.index}", var), L"0");
        EXPECT_EQ(PAPILIO_NS format(L"{.value:*>4}", var), L"**42");
        EXPECT_EQ(PAPILIO_NS format(L"{[0]}", var), L"42");
        EXPECT_THROW((void)PAPILIO_NS format(L"{[1]}", var), format_error);
        EXPECT_EQ(PAPILIO_NS format(L"{[-2]}", var), L"42");
        EXPECT_THROW((void)PAPILIO_NS format(L"{[2]}", var), format_error);
        EXPECT_THROW((void)PAPILIO_NS format(L"{[-3]}", var), format_error);

        var.emplace<float>(3.14f);

        EXPECT_EQ(PAPILIO_NS format("{.index}", var), "1");
        EXPECT_THROW((void)PAPILIO_NS format("{[0]}", var), format_error);
        EXPECT_EQ(PAPILIO_NS format("{[1]}", var), "3.14");
        EXPECT_EQ(PAPILIO_NS format("{[-1]}", var), "3.14");

        EXPECT_EQ(PAPILIO_NS format(L"{.index}", var), L"1");
        EXPECT_THROW((void)PAPILIO_NS format(L"{[0]}", var), format_error);
        EXPECT_EQ(PAPILIO_NS format(L"{[1]}", var), L"3.14");
        EXPECT_EQ(PAPILIO_NS format(L"{[-1]}", var), L"3.14");
    }
}

#ifdef PAPILIO_HAS_LIB_EXPECTED

TEST(accessor, expected)
{
    using namespace papilio;

    {
        std::expected<std::string, int> ex = "hello";

        EXPECT_EQ(PAPILIO_NS format("{.has_value}", ex), "true");
        EXPECT_EQ(PAPILIO_NS format("{.value}", ex), "hello");
        EXPECT_EQ(PAPILIO_NS format("{.value:*^9}", ex), "**hello**");
        EXPECT_THROW((void)PAPILIO_NS format("{.error}", ex), format_error);

        ex = std::unexpected(-1);

        EXPECT_EQ(PAPILIO_NS format("{.has_value}", ex), "false");
        EXPECT_THROW((void)PAPILIO_NS format("{.value}", ex), format_error);
        EXPECT_EQ(PAPILIO_NS format("{.error}", ex), "-1");
        EXPECT_EQ(PAPILIO_NS format("{.error:>5}", ex), "   -1");
    }

    {
        std::expected<std::wstring, int> ex = L"hello";

        EXPECT_EQ(PAPILIO_NS format(L"{.has_value}", ex), L"true");
        EXPECT_EQ(PAPILIO_NS format(L"{.value}", ex), L"hello");
        EXPECT_EQ(PAPILIO_NS format(L"{.value:*^9}", ex), L"**hello**");
        EXPECT_THROW((void)PAPILIO_NS format(L"{.error}", ex), format_error);

        ex = std::unexpected(-1);

        EXPECT_EQ(PAPILIO_NS format(L"{.has_value}", ex), L"false");
        EXPECT_THROW((void)PAPILIO_NS format(L"{.value}", ex), format_error);
        EXPECT_EQ(PAPILIO_NS format(L"{.error}", ex), L"-1");
        EXPECT_EQ(PAPILIO_NS format(L"{.error:>5}", ex), L"   -1");
    }
}

#endif

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
