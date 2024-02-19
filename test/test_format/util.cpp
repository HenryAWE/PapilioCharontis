#include <gtest/gtest.h>
#include "test_format.hpp"

TEST(disabled_formatter, disable_format)
{
    using namespace papilio;
    using test_format::format_disabled;

    static_assert(!formattable<format_disabled>);
    static_assert(!formattable<format_disabled, wchar_t>);

    EXPECT_THROW(
        (void)PAPILIO_NS format("{}", format_disabled()),
        format_error
    );
    EXPECT_THROW(
        (void)PAPILIO_NS format(L"{}", format_disabled()),
        format_error
    );
}
