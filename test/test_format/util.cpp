#include <gtest/gtest.h>
#include "test_format.hpp"

namespace test_format
{
class format_disabled
{
public:
    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, format_disabled)
    {
        os << PAPILIO_TSTRING(CharT, "format disabled");
        return os;
    }
};
} // namespace test_format

namespace papilio
{
template <typename CharT>
struct formatter<test_format::format_disabled, CharT> : public disabled_formatter
{};
} // namespace papilio

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
