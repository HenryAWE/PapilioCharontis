#pragma once

#include <format>
#include <string>
#include <variant>
#include <concepts>
#include <stdexcept>
#include <vector>
#include <map>
#include <charconv>
#include "utf8.hpp"
#include "error.hpp"


namespace papilio
{
    namespace detail
    {
        template <typename CharT>
        [[nodiscard]]
        constexpr bool is_digit(CharT ch) noexcept
        {
            return '0' <= ch && ch <= '9';
        }

        template <typename CharT>
        [[nodiscard]]
        constexpr bool is_identifier_ch(CharT ch, bool first = false) noexcept
        {
            bool digit = is_digit(ch);
            if(first && digit)
                return false;

            return ('A' <= ch && ch <= 'z') || digit || ch == '_';
        }
    }

    template <typename T, typename CharT>
    concept basic_string_like =
        std::is_same_v<std::decay_t<T>, CharT*> ||
        std::is_same_v<std::decay_t<T>, const CharT*> ||
        std::is_same_v<T, std::basic_string<CharT>> ||
        std::is_same_v<T, std::basic_string_view<CharT>>;
    template <typename T>
    concept string_like = basic_string_like<T, char>;

    enum class align : std::uint8_t
    {
        left,
        middle,
        right
    };

    template <typename T>
    struct named_arg
    {
        const char* name;
        const T& value;

        named_arg() = delete;
        constexpr named_arg(const named_arg&) noexcept = default;
        constexpr named_arg(const char* name_, const T& value_) noexcept
            : name(name_), value(value_) {}
    };

    template <typename T>
    constexpr named_arg<T> arg(const char* name, const T& value) noexcept
    {
        return named_arg<T>(name, value);
    }

    inline namespace literals
    {
        struct named_arg_proxy
        {
            const char* name;

            constexpr named_arg_proxy(const char* name_) noexcept
                : name(name_) {}

            template <typename T>
            constexpr named_arg<T> operator=(const T& value) noexcept
            {
                return named_arg(name, value);
            }
        };

        [[nodiscard]]
        constexpr named_arg_proxy operator""_a(const char* name, std::size_t) noexcept
        {
            return named_arg_proxy(name);
        }
    }

    class indexing_value
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using underlying_type = std::variant<
            std::size_t,
            string_type
        >;

        indexing_value() = delete;
        indexing_value(const indexing_value&) = default;
        indexing_value(indexing_value&&) = default;
        indexing_value(std::size_t index)
            : m_val(index) {}
        indexing_value(std::string key)
            : m_val(std::move(key)) {}

        bool is_index() const
        {
            return m_val.index() == 0;
        }
        bool is_key() const
        {
            return m_val.index() == 1;
        }

        std::size_t as_index() const noexcept
        {
            // use std::get_if to avoid exception
            return *std::get_if<std::size_t>(&m_val);
        }
        const std::string& as_key() const noexcept
        {
            return *std::get_if<std::string>(&m_val);
        }

    private:
        underlying_type m_val;
    };
    class attribute_name
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        attribute_name() = delete;
        attribute_name(const attribute_name&) = default;
        attribute_name(attribute_name&&) noexcept = default;
        template <typename... Args> requires(std::constructible_from<string_type, Args...>)
        attribute_name(Args&&... args)
            : m_name(std::forward<Args>(args)...) {}

        friend bool operator==(const attribute_name& lhs, string_view_type rhs)
        {
            return lhs.m_name == rhs;
        }
        friend bool operator==(string_view_type lhs, const attribute_name& rhs)
        {
            return lhs == rhs.m_name;
        }

        [[nodiscard]]
        static bool validate(string_view_type name) noexcept
        {
            bool first = true;
            for(char_type c : name)
            {
                if(!detail::is_identifier_ch(c, first))
                    return false;
                if(first)
                    first = false;
            }

            return true;
        }

    private:
        string_type m_name;
    };


    class format_parse_context
    {
    public:
        using char_type = char;

    private:
    };

    template <std::integral T>
    class formatter
    {
    public:
        using value_type = T;
        using char_type = char;
        using iterator = std::basic_string_view<char_type>::iterator;
        using const_iterator = std::basic_string_view<char_type>::const_iterator;

        void parse(format_parse_context ctx)
        {

        }

        template <typename FormatContext>
        void format(T val, FormatContext& ctx)
        {
            char_type buf[100];
            auto result = std::to_chars(buf, buf + 100, 10);

            if(result.ec)
            {
                throw std::system_error(result.ec);
            }

            ctx.advance_to(std::copy(buf, result.ptr, ctx.out()));
        }

    private:
        int m_base;
    };

    class format_arg
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        class handle
        {
        public:
        private:
        };

        using underlying_type = std::variant<
            std::monostate,
            char_type,
            int,
            unsigned int,
            long long int,
            unsigned long long int,
            float,
            double,
            long double,
            const char_type*,
            std::basic_string_view<char_type>
        >;

        format_arg()
            : m_val() {}
        format_arg(underlying_type val)
            : m_val(std::move(val)) {}

        format_arg index(indexing_value idx)
        {
            auto visitor = [&](auto&& v)->format_arg
            {
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(string_like<T>)
                {
                    if(!idx.is_index())
                        invalid_index();
                    auto i = idx.as_index();

                    string_view_type sv(v);
                    if(i >= sv.size())
                        invalid_index();
                    return format_arg(sv[i]);
                }

                invalid_index();
            };
            return std::visit(visitor, m_val);
        }
        format_arg attribute(attribute_name name)
        {
            auto visitor = [&](auto&& v)->format_arg
            {
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(string_like<T>)
                {
                    string_view_type sv(v);
                    if(name == "length")
                    {
                        return format_arg(utf8::strlen(sv));
                    }
                    else if(name == "size")
                    {
                        return format_arg(sv.size());
                    }
                }

                invalid_attribute();
            };
            return std::visit(visitor, m_val);
        }

        [[noreturn]]
        static void invalid_index()
        {
            throw std::out_of_range("invalid index");
        }
        [[noreturn]]
        static void invalid_attribute()
        {
            throw std::invalid_argument("invalid attribute");
        }

        template <typename T>
        friend const T& get(const format_arg& val)
        {
            return std::get<T>(val.m_val);
        }

    private:
        underlying_type m_val;
    };

    class dynamic_format_arg_store
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        template <typename... Args>
        dynamic_format_arg_store(Args&&... args)
        {
            
        }

        const format_arg& get(std::size_t i)
        {
            if(i >= m_args.size())
                throw std::out_of_range("index out of range");
            return m_args[i];
        }
        const format_arg& get(string_view_type key)
        {
            auto it = m_named_args.find(key);
            if(it == m_named_args.end())
                throw std::out_of_range("invalid named argument");
            return it->second;
        }

    private:
        std::vector<format_arg> m_args;
        std::map<string_type, format_arg, std::less<>> m_named_args;
    };


    template <typename OutputIt>
    class format_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = OutputIt;

        iterator out()
        {
            return std::move(m_out);
        }
        void advance_to(iterator it)
        {
            m_out = it;
        }

    private:
        iterator m_out;
        
    };

    namespace detail
    {
        template <typename T, typename CharT = char>
        concept has_std_formatter = requires
        {
            typename std::formatter<T, CharT>;
        };

        struct format_spec
        {

        };
    }

    
}
