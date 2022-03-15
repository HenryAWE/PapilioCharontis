#pragma once

#include <string>


namespace papilio
{
    enum class block_type : int
    {
        text,
        relpacement_field,
        script
    };

    template <typename CharT>
    class basic_block
    {
    public:
        typedef CharT value_type;
        typedef std::basic_string<CharT> string_type;

        basic_block(block_type type_, string_type str_)
            : m_type(type_), m_str(std::move(str_)) {}

        void assign(block_type type_, string_type str_)
        {
            m_type = type_;
            m_str.swap(str_);
        }

        constexpr block_type type() const noexcept { return m_type; }
        constexpr const string_type& str() const noexcept { return m_str; }

    private:
        block_type m_type;
        string_type m_str;
    };
}
