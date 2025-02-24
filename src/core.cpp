#include <papilio/core.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
format_error::~format_error() = default;

bad_handle_cast::~bad_handle_cast() = default;

namespace detail
{
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
            PAPILIO_SCRIPT_ERR(invalid_fmt_spec, "invalid format specification");
            PAPILIO_SCRIPT_ERR(unenclosed_brace, "unenclosed brace");

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
            PAPILIO_SCRIPT_ERR(invalid_fmt_spec, "invalid format specification");
            PAPILIO_SCRIPT_ERR(unenclosed_brace, "unenclosed brace");

        [[unlikely]] default:
            PAPILIO_SCRIPT_ERR(unknown_error, "unknown error");
        }

#undef PAPILIO_SCRIPT_ERR
    }
} // namespace detail

std::string to_string(script_error_code ec)
{
    return std::string(detail::script_ec_to_sv(ec));
}

std::wstring to_wstring(script_error_code ec)
{
    return std::wstring(detail::script_ec_to_wsv(ec));
}

std::ostream& operator<<(std::ostream& os, script_error_code ec)
{
    os << detail::script_ec_to_sv(ec);
    return os;
}

std::wostream& operator<<(std::wostream& os, script_error_code ec)
{
    os << detail::script_ec_to_wsv(ec);
    return os;
}

bad_variable_access::~bad_variable_access() = default;

invalid_conversion::~invalid_conversion() = default;

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
} // namespace papilio

#include <papilio/detail/suffix.hpp>
