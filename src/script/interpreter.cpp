#include <papilio/script/interpreter.hpp>
#include <papilio/detail/prefix.hpp>
#ifdef PAPILIO_STDLIB_LIBCPP
// from_chars of libc++ is broken, use stringstream as a fallback.
#    include <sstream>
#    define PAPILIO_ENABLE_LIBCPP_CHARCONV_WORKAROUND 1
#endif

namespace papilio::script
{
bad_variable_access::~bad_variable_access() = default;

invalid_conversion::~invalid_conversion() = default;

static std::string_view script_ec_to_sv(script_error_code ec) noexcept
{
    using namespace std::literals;
    using enum script_error_code;

#define PAPILIO_SCRIPT_ERR(code, msg) \
    case code: return msg##sv

    switch(ec)
    {
        PAPILIO_SCRIPT_ERR(no_error, "no error");
        PAPILIO_SCRIPT_ERR(end_of_string, "end of string");
        PAPILIO_SCRIPT_ERR(invalid_field_name, "invalid field name");
        PAPILIO_SCRIPT_ERR(invalid_condition, "invalid condition");
        PAPILIO_SCRIPT_ERR(invalid_index, "invalid index");
        PAPILIO_SCRIPT_ERR(invalid_attribute, "invalid attribute");
        PAPILIO_SCRIPT_ERR(invalid_operator, "invalid operator");
        PAPILIO_SCRIPT_ERR(invalid_string, "invalid string");
        PAPILIO_SCRIPT_ERR(unclosed_brace, "unclosed brace");

    [[unlikely]] default:
        PAPILIO_SCRIPT_ERR(unknown_error, "unknown error");
    }

#undef PAPILIO_SCRIPT_ERR
}

static std::wstring_view script_ec_to_wsv(script_error_code ec) noexcept
{
    using namespace std::literals;
    using enum script_error_code;

#define PAPILIO_SCRIPT_ERR(code, msg) \
    case code: return L##msg##sv

    switch(ec)
    {
        PAPILIO_SCRIPT_ERR(no_error, "no error");
        PAPILIO_SCRIPT_ERR(end_of_string, "end of string");
        PAPILIO_SCRIPT_ERR(invalid_field_name, "invalid field name");
        PAPILIO_SCRIPT_ERR(invalid_condition, "invalid condition");
        PAPILIO_SCRIPT_ERR(invalid_index, "invalid index");
        PAPILIO_SCRIPT_ERR(invalid_attribute, "invalid attribute");
        PAPILIO_SCRIPT_ERR(invalid_operator, "invalid operator");
        PAPILIO_SCRIPT_ERR(invalid_string, "invalid string");
        PAPILIO_SCRIPT_ERR(unclosed_brace, "unclosed brace");

    [[unlikely]] default:
        PAPILIO_SCRIPT_ERR(unknown_error, "unknown error");
    }

#undef PAPILIO_SCRIPT_ERR
}

std::string to_string(script_error_code ec)
{
    return std::string(script_ec_to_sv(ec));
}

std::wstring to_wstring(script_error_code ec)
{
    return std::wstring(script_ec_to_wsv(ec));
}

std::ostream& operator<<(std::ostream& os, script_error_code ec)
{
    os << script_ec_to_sv(ec);
    return os;
}

std::wostream& operator<<(std::wostream& os, script_error_code ec)
{
    os << script_ec_to_wsv(ec);
    return os;
}

script_base::error::error(script_error_code ec)
    : format_error(to_string(ec)), m_ec(ec) {}

script_base::error::~error() = default;

script_base::error script_base::make_error(script_error_code ec)
{
    return error(ec);
}

void script_base::throw_end_of_string()
{
    throw make_error(script_error_code::end_of_string);
}

void script_base::throw_error(script_error_code ec)
{
    throw make_error(ec);
}

bool script_base::is_op_ch(char32_t ch) noexcept
{
    return ch == U'=' ||
           ch == U'!' ||
           ch == U'>' ||
           ch == U'<';
}

bool script_base::is_var_start_ch(char32_t ch) noexcept
{
    return ch == U'\'' ||
           ch == U'{' ||
           PAPILIO_NS utf::is_digit(ch) ||
           ch == U'-' ||
           ch == U'.';
}

bool script_base::is_field_name_ch(char32_t ch, bool first) noexcept
{
    bool digit = PAPILIO_NS utf::is_digit(ch);
    if(digit && first)
        return false;

    return digit ||
           (U'A' <= ch && ch <= U'A') ||
           (U'a' <= ch && ch <= U'z') ||
           ch == U'_' ||
           ch >= 128;
}

bool script_base::is_field_name_end_ch(char32_t ch) noexcept
{
    return ch == U'}' ||
           ch == U':' ||
           ch == U'.' ||
           ch == U'[';
}

char32_t script_base::get_esc_ch(char32_t ch) noexcept
{
    switch(ch)
    {
    case U'n':
        return U'\n';
    case U't':
        return U'\t';

    case U'\'':
    default:
        return ch;
    }
}

float script_base::chars_to_float(const char* start, const char* stop)
{
#ifndef PAPILIO_ENABLE_LIBCPP_CHARCONV_WORKAROUND
    // Ordinary implementation using <charconv>

    float val;
    auto result = std::from_chars(
        start, stop, val, std::chars_format::fixed
    );
    if(result.ec != std::errc())
    {
        throw std::runtime_error("invalid float");
    }

    return val;

#else
    // PAPILIO_ENABLE_LIBCPP_CHARCONV_WORKAROUND is defined
    // Workaround for libc++ using <sstream>

    std::stringstream ss;
    ss.imbue(std::locale::classic());
    ss.write(start, stop - start);

    float val;
    ss >> val;

    if(!ss.good())
    {
        throw std::runtime_error("invalid float");
    }

    return val;

#endif
}
} // namespace papilio::script

#include <papilio/detail/suffix.hpp>
