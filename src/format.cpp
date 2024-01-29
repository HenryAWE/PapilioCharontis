#include <papilio/format.hpp>

namespace papilio
{
std::string vformat(
    std::string_view fmt, const format_args_ref& args
)
{
    std::string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

std::string vformat(
    const std::locale& loc, std::string_view fmt, const format_args_ref& args
)
{
    std::string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), loc, fmt, args);

    return result;
}

std::wstring vformat(
    std::wstring_view fmt, const wformat_args_ref& args
)
{
    std::wstring result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, args);

    return result;
}

std::wstring vformat(
    const std::locale& loc, std::wstring_view fmt, const wformat_args_ref& args
)
{
    std::wstring result;
    PAPILIO_NS vformat_to(std::back_inserter(result), loc, fmt, args);

    return result;
}
} // namespace papilio
