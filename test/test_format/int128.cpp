#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <papilio/formatter/int128.hpp>
#include <papilio_test/setup.hpp>

#ifdef PAPILIO_HAS_INT128

namespace test_format
{
#    ifdef PAPILIO_IMPL_INT128_MSVC_STL

// == UINT64_MAX
static constexpr papilio::int128_t i128_data_a()
{
    return std::_Signed128(
        std::numeric_limits<std::uint64_t>::max(), 0uLL
    );
}

// == UINT64_MAX + 1
static constexpr papilio::int128_t i128_data_b()
{
    auto tmp = std::_Signed128(
        std::numeric_limits<std::uint64_t>::max(), 0uLL
    );
    return tmp + 1;
}

#    endif
} // namespace test_format

TEST(int128_formatter, int128)
{
    using namespace test_format;
    using namespace papilio;

    static_assert(formattable<papilio::int128_t>);

    // UINT64_MAX
    EXPECT_EQ(
        PAPILIO_NS format("{:d}", i128_data_a()),
        "18446744073709551615"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:d}", i128_data_a()),
        L"18446744073709551615"
    );

    // UINT64_MAX + 1
    EXPECT_EQ(
        PAPILIO_NS format("{:d}", i128_data_b()),
        "18446744073709551616"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:d}", i128_data_b()),
        L"18446744073709551616"
    );
}

namespace test_format
{
#    ifdef PAPILIO_IMPL_INT128_MSVC_STL

// == UINT64_MAX
static constexpr papilio::uint128_t u128_data_a()
{
    return std::_Unsigned128(
        std::numeric_limits<std::uint64_t>::max(), 0uLL
    );
}

// == UINT64_MAX + 1
static constexpr papilio::uint128_t u128_data_b()
{
    auto tmp = std::_Unsigned128(
        std::numeric_limits<std::uint64_t>::max(), 0uLL
    );
    return tmp + 1;
}

#    endif
} // namespace test_format

TEST(int128_formatter, uint128)
{
    using namespace test_format;
    using namespace papilio;

    static_assert(formattable<papilio::uint128_t>);

    // UINT64_MAX
    EXPECT_EQ(
        PAPILIO_NS format("{:d}", u128_data_a()),
        "18446744073709551615"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:d}", u128_data_a()),
        L"18446744073709551615"
    );

    // UINT64_MAX + 1
    EXPECT_EQ(
        PAPILIO_NS format("{:d}", u128_data_b()),
        "18446744073709551616"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:d}", u128_data_b()),
        L"18446744073709551616"
    );
}

#endif
