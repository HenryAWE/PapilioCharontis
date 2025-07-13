#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <papilio/formatter/int128.hpp>
#include <papilio_test/setup.hpp>

#ifdef PAPILIO_HAS_INT128

#    ifdef PAPILIO_COMPILER_GCC
// Ignoring "ISO C++ does not support '__int128' for 'type name'"
#        pragma GCC diagnostic ignored "-Wpedantic"
#    endif

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

#    ifdef PAPILIO_IMPL_INT128_EXT__INT128

// == UINT64_MAX
static constexpr papilio::int128_t i128_data_a()
{
    return static_cast<__int128>(
        std::numeric_limits<std::uint64_t>::max()
    );
}

// == UINT64_MAX + 1
static constexpr papilio::int128_t i128_data_b()
{
    auto tmp = static_cast<__int128>(
        std::numeric_limits<std::uint64_t>::max()
    );
    return tmp + 1;
}

#    endif
} // namespace test_format

TEST(int128_formatter, int128)
{
    using namespace test_format;
    using namespace papilio;

    static_assert(papilio::integral_128bit<papilio::int128_t>);
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

#    ifdef PAPILIO_IMPL_INT128_EXT__INT128

// == UINT64_MAX
static constexpr papilio::uint128_t u128_data_a()
{
    return static_cast<unsigned __int128>(
        std::numeric_limits<std::uint64_t>::max()
    );
}

// == UINT64_MAX + 1
static constexpr papilio::uint128_t u128_data_b()
{
    auto tmp = static_cast<unsigned __int128>(
        std::numeric_limits<std::uint64_t>::max()
    );
    return tmp + 1;
}

#    endif

static papilio::uint128_t uint128_max()
{
    return ~papilio::uint128_t(0);
}
} // namespace test_format

TEST(int128_formatter, uint128)
{
    using namespace test_format;
    using namespace papilio;

    static_assert(papilio::integral_128bit<papilio::uint128_t>);
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

    // UINT128_MAX
    EXPECT_EQ(
        PAPILIO_NS format("{}", uint128_max()),
        "340282366920938463463374607431768211455"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{}", uint128_max()),
        L"340282366920938463463374607431768211455"
    );
    EXPECT_EQ(
        PAPILIO_NS format("{:x}", uint128_max()),
        "ffffffffffffffffffffffffffffffff"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:x}", uint128_max()),
        L"ffffffffffffffffffffffffffffffff"
    );
}

#endif
