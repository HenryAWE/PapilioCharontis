#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <papilio/formatter/filesystem.hpp>
#include <vector>
#include <iostream>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(formatter, filesystem_path)
{
    using namespace papilio;

    {
        std::filesystem::path empty;
        EXPECT_EQ(PAPILIO_NS format("{}", empty), "");
        EXPECT_EQ(PAPILIO_NS format(L"{}", empty), L"");
    }

    {
        std::filesystem::path p = "folder";
        p /= "file.txt";
        EXPECT_EQ(p, "folder/file.txt");

        EXPECT_EQ(PAPILIO_NS format("{:g}", p), "folder/file.txt");
        EXPECT_EQ(PAPILIO_NS format(L"{:g}", p), L"folder/file.txt");
        EXPECT_EQ(PAPILIO_NS format("{:?g}", p), R"("folder/file.txt")");
        EXPECT_EQ(PAPILIO_NS format(L"{:?g}", p), LR"("folder/file.txt")");

        EXPECT_EQ(PAPILIO_NS format("{:*^21g}", p), "***folder/file.txt***");
        EXPECT_EQ(PAPILIO_NS format(L"{:*^21g}", p), L"***folder/file.txt***");
    }

    {
        std::filesystem::path p = "folder";
        p /= "file.txt";
        EXPECT_EQ(p, "folder/file.txt");

#ifdef PAPILIO_PLATFORM_WINDOWS
        static_assert(std::filesystem::path::preferred_separator == L'\\');

        EXPECT_EQ(PAPILIO_NS format("{}", p), "folder\\file.txt");
        EXPECT_EQ(PAPILIO_NS format(L"{}", p), L"folder\\file.txt");

        EXPECT_EQ(PAPILIO_NS format("{:?}", p), R"("folder\\file.txt")");
        EXPECT_EQ(PAPILIO_NS format(L"{:?}", p), LR"("folder\\file.txt")");

#else
        static_assert(std::filesystem::path::preferred_separator == '/');

        EXPECT_EQ(PAPILIO_NS format("{}", p), "folder/file.txt");
        EXPECT_EQ(PAPILIO_NS format(L"{}", p), L"folder/file.txt");

        EXPECT_EQ(PAPILIO_NS format("{:?}", p), R"("folder/file.txt")");
        EXPECT_EQ(PAPILIO_NS format(L"{:?}", p), LR"("folder/file.txt")");
#endif
    }

#ifdef PAPILIO_PLATFORM_WINDOWS
    {
        static_assert(std::same_as<std::filesystem::path::value_type, wchar_t>);
        static_assert(std::filesystem::path::preferred_separator == L'\\');

        // Test Chinese path
        std::filesystem::path non_ascii = L"中文路径";
        non_ascii /= L"文件.txt";
        EXPECT_EQ(non_ascii, L"中文路径/文件.txt");

        EXPECT_EQ(PAPILIO_NS format("{}", non_ascii), "中文路径\\文件.txt");
        EXPECT_EQ(PAPILIO_NS format(L"{}", non_ascii), L"中文路径\\文件.txt");
        EXPECT_EQ(PAPILIO_NS format("{:g}", non_ascii), "中文路径/文件.txt");
        EXPECT_EQ(PAPILIO_NS format(L"{:g}", non_ascii), L"中文路径/文件.txt");
    }
#endif
}
