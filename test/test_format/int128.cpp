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

static papilio::int128_t i128_min()
{
    return std::numeric_limits<papilio::int128_t>::min();
}

static papilio::int128_t i128_max()
{
    return std::numeric_limits<papilio::int128_t>::max();
}
} // namespace test_format

TEST(Int128Formatter, Int128)
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

    // INT128_MIN
    EXPECT_EQ(
        PAPILIO_NS format("{}", i128_min()),
        "-170141183460469231731687303715884105728"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{}", i128_min()),
        L"-170141183460469231731687303715884105728"
    );
    EXPECT_EQ(
        PAPILIO_NS format("{:x}", i128_min()),
        "-80000000000000000000000000000000"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:x}", i128_min()),
        L"-80000000000000000000000000000000"
    );

    // INT128_MAX
    EXPECT_EQ(
        PAPILIO_NS format("{}", i128_max()),
        "170141183460469231731687303715884105727"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{}", i128_max()),
        L"170141183460469231731687303715884105727"
    );
    EXPECT_EQ(
        PAPILIO_NS format("{:x}", i128_max()),
        "7fffffffffffffffffffffffffffffff"
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{:x}", i128_max()),
        L"7fffffffffffffffffffffffffffffff"
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

TEST(Int128Formatter, UInt128)
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
