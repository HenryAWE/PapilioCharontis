#include <papilio/script/interpreter.hpp>

namespace papilio::script
{
std::string to_string(interpreter_base::script_error_code ec)
{
    using enum interpreter_base::script_error_code;

#define PAPILIO_SCRIPT_ERR(code, msg) \
    case code: return msg

    switch(ec)
    {
        PAPILIO_SCRIPT_ERR(end_of_string, "end of string");
        PAPILIO_SCRIPT_ERR(invalid_field_name, "invalid field name");
        PAPILIO_SCRIPT_ERR(invalid_condition, "invalid condition");
        PAPILIO_SCRIPT_ERR(invalid_index, "invalid index");
        PAPILIO_SCRIPT_ERR(invalid_attribute, "invalid attribute");
        PAPILIO_SCRIPT_ERR(invalid_operator, "invalid operator");
        PAPILIO_SCRIPT_ERR(invalid_string, "invalid string");
        PAPILIO_SCRIPT_ERR(unclosed_brace, "unclosed brace");

    [[unlikely]] default:
        throw std::invalid_argument("invalid error code");
    }
}

std::ostream& operator<<(std::ostream& os, interpreter_base::script_error_code ec)
{
    os << to_string(ec);
    return os;
}

interpreter_base::script_error::script_error(script_error_code ec)
    : format_error(to_string(ec)), m_ec(ec) {}

void interpreter_base::throw_end_of_string()
{
    throw script_error(script_error_code::end_of_string);
}

void interpreter_base::throw_error(script_error_code ec)
{
    throw script_error(ec);
}
} // namespace papilio::script
