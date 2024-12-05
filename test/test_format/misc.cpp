#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include <papilio/print.hpp>
#include <papilio/formatter/misc.hpp>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(misc_formatter, stream_adaptor)
{
    using namespace papilio;
    using test_format::stream_only;

    {
        EXPECT_EQ(
            PAPILIO_NS format("{}", stream_only{}),
            "stream only"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{}", stream_only{}),
            L"stream only"
        );
    }

    {
        EXPECT_EQ(
            PAPILIO_NS format("{:>15}", stream_only{}),
            "    stream only"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{:>15}", stream_only{}),
            L"    stream only"
        );
    }
}

TEST(misc_formatter, join)
{
    using namespace papilio;

    {
        int arr[4] = {1, 2, 3, 4};
        EXPECT_EQ(
            PAPILIO_NS format("{}", PAPILIO_NS join(arr)),
            "1, 2, 3, 4"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{}", PAPILIO_NS join<wchar_t>(arr)),
            L"1, 2, 3, 4"
        );
    }

    {
        int arr[4] = {1, 2, 3, 4};
        EXPECT_EQ(
            PAPILIO_NS format("{}", PAPILIO_NS join(arr, " | ")),
            "1 | 2 | 3 | 4"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{}", PAPILIO_NS join(arr, L" | ")),
            L"1 | 2 | 3 | 4"
        );
    }

    {
        int arr[4] = {1, 2, 3, 4};

        static_assert(formattable<decltype(join(arr, "|"))>);
        static_assert(formattable<decltype(join(arr, L"|")), wchar_t>);

        EXPECT_EQ(
            PAPILIO_NS format("{:02}", PAPILIO_NS join(arr, " | ")),
            "01 | 02 | 03 | 04"
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{:02}", PAPILIO_NS join(arr, L" | ")),
            L"01 | 02 | 03 | 04"
        );
    }
}

TEST(misc_formatter, thread_id)
{
    using namespace papilio;

    auto id = std::this_thread::get_id();

    {
        const std::string expected_str = [&]()
        {
            std::stringstream ss;
            ss << id;
            return std::move(ss).str();
        }();

        EXPECT_EQ(PAPILIO_NS format("{}", id), expected_str);
    }

    {
        const std::string expected_str = [&]()
        {
            std::stringstream ss;
            ss << id;
            return PAPILIO_NS format("{:*^20}", std::move(ss).str());
        }();

        std::string result = PAPILIO_NS format("{:*^20}", id);

        EXPECT_EQ(result, expected_str);
        // Print platform-dependent result for visual check
        PAPILIO_NS println(std::cerr, "result = \"{}\"", result);
    }

    {
        const std::wstring wexpected_str = [&]()
        {
            std::wstringstream ss;
            ss << id;
            return std::move(ss).str();
        }();

        EXPECT_EQ(PAPILIO_NS format(L"{}", id), wexpected_str);
    }
}

#ifdef PAPILIO_HAS_LIB_STACKTRACE

TEST(misc_formatter, stacktrace)
{
    using namespace papilio;

    auto cur = std::stacktrace::current();

    {
        const std::string expected_str = std::to_string(cur);

        EXPECT_EQ(
            PAPILIO_NS format("{}", cur),
            expected_str
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{}", cur),
            utf::string_ref(expected_str).to_wstring()
        );

        // Print platform-dependent result for visual check
        PAPILIO_NS println(std::cerr, "std::stacktrace::current():\n{}", cur);
    }

    if(cur.size() == 0)
        GTEST_SKIP() << "cur.size() == 0";

    {
        const std::string expected_str = std::to_string(cur[0]);

        EXPECT_EQ(
            PAPILIO_NS format("{}", cur[0]),
            expected_str
        );
        EXPECT_EQ(
            PAPILIO_NS format("{:*^120}", cur[0]),
            PAPILIO_NS format("{:*^120}", expected_str)
        );
        EXPECT_EQ(
            PAPILIO_NS format(L"{}", cur[0]),
            utf::string_ref(expected_str).to_wstring()
        );

        // Print platform-dependent result for visual check
        PAPILIO_NS println(std::cerr, "cur[0]:\n{}", cur[0]);
        PAPILIO_NS println(std::cerr, "cur[0] {{:*^120}}:\n{:*^120}", cur[0]);
    }
}

#endif
