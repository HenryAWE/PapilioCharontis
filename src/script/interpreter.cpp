#include <papilio/script/interpreter.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio::script
{
static const char* to_cstr(script_error_code ec) noexcept
{
    using enum script_error_code;

#define PAPILIO_SCRIPT_ERR(code, msg) \
    case code: return msg

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

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

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif
}

std::string to_string(script_error_code ec)
{
    return to_cstr(ec);
}

std::ostream& operator<<(std::ostream& os, script_error_code ec)
{
    os << to_cstr(ec);
    return os;
}

script_base::error::error(script_error_code ec)
    : format_error(to_string(ec)), m_ec(ec) {}

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
} // namespace papilio::script

#include <papilio/detail/suffix.hpp>
