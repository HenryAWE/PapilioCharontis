#include <gtest/gtest.h>
#include <papilio/format.hpp>

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
        const std::wstring wexpected_str = [&]()
        {
            std::wstringstream ss;
            ss << id;
            return std::move(ss).str();
        }();

        EXPECT_EQ(PAPILIO_NS format(L"{}", id), wexpected_str);
    }
}

TEST(misc_formatter, stacktrace)
{
    using namespace papilio;

#ifndef PAPILIO_HAS_STACKTRACE
    GTEST_SKIP() << "No <stacktrace> support";

#else
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
            PAPILIO_NS format(L"{}", cur[0]),
            utf::string_ref(expected_str).to_wstring()
        );
    }
#endif
}
