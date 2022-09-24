#pragma once

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <span>
#include <algorithm>
#include <utility>
#include <cassert>
#include <charconv>
#include <optional>
#include "error.hpp"


namespace papilio
{
    class dynamic_format_arg_store;

    namespace script
    {
        class script_context;

        class error : public std::runtime_error
        {
        public:
            using runtime_error::runtime_error;
        };
        class parse_error : public error
        {
        public:
            using error::error;
        };
        class lexer_error : public parse_error
        {
        public:
            using parse_error::parse_error;
        };
        class invalid_argument_name : public lexer_error
        {
        public:
            invalid_argument_name(std::string bad_name)
                : lexer_error("invalid argument name: " + bad_name),
                m_bad_name(std::move(bad_name)) {}

            const std::string& bad_name() noexcept
            {
                return m_bad_name;
            }

        private:
            std::string m_bad_name;
        };

        namespace detail
        {
            [[nodiscard]]
            constexpr bool is_space(char ch) noexcept
            {
                return
                    ch == ' ' ||
                    ch == '\n' ||
                    ch == '\t' ||
                    ch == '\v' ||
                    ch == '\f';
            }
            [[nodiscard]]
            constexpr bool is_digit(char ch) noexcept
            {
                return '0' <= ch && ch <= '9';
            }
            [[nodiscard]]
            constexpr bool is_xdigit(char ch) noexcept
            {
                return
                    is_digit(ch) ||
                    ('a' <= ch && ch <= 'f') ||
                    ('A' <= ch && ch <= 'F');
            }
            [[nodiscard]]
            constexpr bool is_alpha(char ch) noexcept
            {
                return
                    ('A' <= ch && ch <= 'Z') ||
                    ('a' <= ch && ch <= 'z');
            }
            [[nodiscard]]
            constexpr bool is_identifier(char ch, bool first) noexcept
            {
                bool digit = is_digit(ch);
                if(first && digit)
                    return false;
                return
                    is_alpha(ch) ||
                    digit ||
                    ch == '_';
            }
            struct is_identifier_helper
            {
                bool first = true;

                [[nodiscard]]
                constexpr bool operator()(char ch) noexcept
                {
                    bool result = is_identifier(ch, first);
                    first = false;
                    return result;
                }
            };
            
            class script_op_base
            {
            public:
                virtual ~script_op_base() = default;

                virtual void execute(script_context& ctx);
            };
        }

        class script_context
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;

        private:
        };

        class script
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;
            using string_view_type = std::basic_string_view<char_type>;

            void execute(script_context& ctx);

        private:
            std::unique_ptr<detail::script_op_base> m_scirpt_ops;
        };

        enum class lexeme_type
        {
            argument,
            identifier,
            constant,
            keyword,
            operator_,
            field   // replacement field
        };

        enum class keyword_type
        {
            if_,
            elif,
            else_
        };

        enum class operator_type
        {
            colon,  // :
            comma,  // ,
            dot,    // .
            parenthesis_l,  // (
            parenthesis_r,  // )
            bracket_l,  // [
            bracket_r,   // ]
            not_,   // !
            and_,   // &&
            or_,    // ||
            equal, // ==
            not_equal,  // !=
            greater_than,   // >
            less_than,  // <
            greater_equal,  // >=
            less_equal, // <=
        };

        template <typename T>
        concept is_lexeme = requires(T t)
        {
            { t.type }->std::same_as<const lexeme_type&>;
        };

        class lexeme
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;
            using string_view_type = std::basic_string_view<char_type>;

            class argument
            {
            public:
                using index_type = std::size_t;
                using underlying_type = std::variant<index_type, string_type>;

                static constexpr lexeme_type type = lexeme_type::argument;

                argument() = delete;
                argument(const argument&) = default;
                argument(argument&&) = default;
                constexpr argument(index_type idx) noexcept
                    : m_arg(idx) {}
                argument(string_type name)
                    : m_arg(std::move(name)) {}
                
                [[nodiscard]]
                bool is_indexed() const noexcept
                {
                    return m_arg.index() == 0;
                }
                [[nodiscard]]
                bool is_named() const noexcept
                {
                    return m_arg.index() == 1;
                }

                template <typename T>
                [[nodiscard]]
                bool holds() const noexcept
                {
                    return std::holds_alternative<T>(m_arg);
                }

                [[nodiscard]]
                index_type get_index() const noexcept
                {
                    assert(holds<index_type>());
                    return *std::get_if<index_type>(&m_arg);
                }
                [[nodiscard]]
                const string_type& get_string() const noexcept
                {
                    assert(holds<string_type>());
                    return *std::get_if<string_type>(&m_arg);
                }

            private:
                underlying_type m_arg;
            };

            class identifier
            {
            public:
                static constexpr lexeme_type type = lexeme_type::identifier;

                identifier() = delete;
                identifier(const identifier&) = default;
                identifier(identifier&&) = default;
                identifier(string_type str)
                    : m_str(std::move(str)) {}
                identifier(string_view_type sv)
                    : m_str(sv) {}

                [[nodiscard]]
                const string_type& get() const noexcept
                {
                    return m_str;
                }

            private:
                string_type m_str;
            };

            class constant
            {
            public:
                using int_type = std::int64_t;
                using float_type = long double;
                using underlying_type = std::variant<
                    int_type,
                    float_type,
                    string_type
                >;

                static constexpr lexeme_type type = lexeme_type::constant;

                constant() = delete;
                constant(const constant&) = default;
                constant(constant&&) noexcept = default;
                constant(int_type i)
                    : m_const(i) {}
                constant(float_type f)
                    : m_const(f) {}
                constant(string_type str)
                    : m_const(std::move(str)) {}

                template <typename T>
                bool holds() const noexcept
                {
                    return std::holds_alternative<T>(m_const);
                }

                [[nodiscard]]
                int_type get_int() const noexcept
                {
                    assert(holds<int_type>());
                    return *std::get_if<int_type>(&m_const);
                }
                [[nodiscard]]
                float_type get_float() const noexcept
                {
                    assert(holds<float_type>());
                    return *std::get_if<float_type>(&m_const);
                }
                [[nodiscard]]
                const string_type& get_string() const noexcept
                {
                    assert(holds<string_type>());
                    return *std::get_if<string_type>(&m_const);
                }

            private:
                underlying_type m_const;
            };

            class keyword
            {
            public:
                static constexpr lexeme_type type = lexeme_type::keyword;

                keyword() = delete;
                constexpr keyword(const keyword&) noexcept = default;
                constexpr keyword(keyword_type kt) noexcept
                    : m_kw_type(kt) {}

                [[nodiscard]]
                constexpr keyword_type get() const noexcept
                {
                    return m_kw_type;
                }

            private:
                keyword_type m_kw_type;
            };

            class operator_
            {
            public:
                static constexpr lexeme_type type = lexeme_type::operator_;

                operator_() = delete;
                constexpr operator_(const operator_&) noexcept = default;
                constexpr operator_(operator_type opt) noexcept
                    : m_op_type(opt) {}

                [[nodiscard]]
                constexpr operator_type get() const noexcept
                {
                    return m_op_type;
                }

            private:
                operator_type m_op_type;
            };

            // replacement field
            class field
            {
            public:
                static constexpr lexeme_type type = lexeme_type::field;

                field();
                field(const field&) = default;
                field(field&&) noexcept = default;
                field(string_type fmt_str)
                    : m_fmt_str(std::move(fmt_str)) {}
                field(string_view_type fmt_sv)
                    : m_fmt_str(fmt_sv) {}

                const string_type& get() const noexcept
                {
                    return m_fmt_str;
                }

            private:
                string_type m_fmt_str;
            };

            using lexeme_store = std::variant<
                argument,
                identifier,
                constant,
                keyword,
                operator_,
                field
            >;

            lexeme() = delete;
            lexeme(const lexeme&) = default;
            lexeme(lexeme&&) = default;
            template <typename Lexeme> requires is_lexeme<Lexeme>
            lexeme(Lexeme lex)
                : m_store(std::move(lex)) {}

            lexeme_type type() const noexcept
            {
                return std::visit(
                    [](auto&& v) { return v.type; },
                    m_store
                );
            }

            template <typename T> requires is_lexeme<T>
            bool holds() const noexcept
            {
                return std::holds_alternative<T>(m_store);
            }
            template <typename T> requires is_lexeme<T>
            const T& as() const noexcept
            {
                assert(holds<T>());
                return *std::get_if<T>(&m_store);
            }

        private:
            lexeme_store m_store;
        };

        class lexer
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;
            using string_view_type = std::basic_string_view<char_type>;
            using const_iterator =  string_view_type::const_iterator;
            using iterator = const_iterator;

            void parse(string_view_type src);

            std::span<const lexeme> lexemes() const noexcept
            {
                return m_lexemes;
            }

            void clear() noexcept
            {
                m_lexemes.clear();
            }

        private:
            std::vector<lexeme> m_lexemes;

            template <typename Lexeme, typename... Args> requires is_lexeme<Lexeme>
            void push_lexeme(Args&&... args)
            {
                m_lexemes.emplace_back(Lexeme(std::forward<Args>(args)...));
            }

            iterator consume_whitespace(iterator begin, iterator end)
            {
                return std::find_if_not(begin, end, &detail::is_space);
            }

            std::pair<lexeme::constant, iterator> parse_number(iterator begin, iterator end)
            {
                bool dot = false;
                int base = 10;
                bool neg = false;
                string_view_type sv(begin, end);

                if(sv.starts_with("-"))
                {
                    neg = true;
                    begin = std::next(begin);
                }
                
                if(sv.starts_with("0x"))
                {
                    base = 16;
                    begin = std::next(begin, 2);
                }
                else if(sv.starts_with("0o"))
                {
                    base = 8;
                    begin = std::next(begin, 2);
                }
                else if(sv.starts_with("0b"))
                {
                    base = 2;
                    begin = std::next(begin, 2);
                }

                auto next = begin;
                for(; next != end; ++next)
                {
                    char_type ch = *next;
                    if(ch == '.')
                    {
                        if(dot)
                            throw lexer_error("lexer error");
                        dot = true;
                        continue;
                    }
                    
                    switch(base)
                    {
                    case 2:
                        if(ch != '0' && ch != '1')
                            goto end_loop;
                    [[unlikely]] case 8:
                        if(!('0' <= ch && ch <= '7'))
                            goto end_loop;
                    [[likely]] case 10:
                        if(!detail::is_digit(ch))
                            goto end_loop;
                    case 16:
                        if(!detail::is_xdigit(ch))
                            goto end_loop;
                    }
                }
            end_loop:
                if(dot)
                {
                    lexeme::constant::float_type result;
                    auto conv = std::from_chars(
                        std::to_address(begin), std::to_address(next),
                        result,
                        std::chars_format::general
                    );
                    if(conv.ec != std::errc())
                        throw lexer_error("lexer error");
                    assert(conv.ptr == std::to_address(next));
                    if(neg)
                        result = -result;

                    return std::make_pair(result, next);
                }
                else
                {
                    lexeme::constant::int_type result;
                    auto conv = std::from_chars(
                        std::to_address(begin), std::to_address(next),
                        result,
                        base
                    );
                    if(conv.ec != std::errc())
                        throw lexer_error("lexer error");
                    assert(conv.ptr == std::to_address(next));
                    if(neg)
                        result = -result;
                    return std::make_pair(result, next);
                }

                throw lexer_error("lexer error");
            }
            // Note: this function assumes that *--begin == '\''
            std::pair<string_type, iterator> parse_string(iterator begin, iterator end)
            {
                string_type result;

                bool escape = false;
                iterator it = begin;
                for(; it != end; ++it)
                {
                    char_type ch = *it;
                    switch(ch)
                    {
                    default:
                        result += ch;
                        break;

                    case '\\':
                        if(escape)
                        {
                            escape = false;
                            result += '\\';
                        }
                        else
                            escape = true;
                        break;

                    case '\'':
                        if(escape)
                        {
                            escape = false;
                            result += '\'';
                        }
                        else
                            return std::make_pair(std::move(result), std::next(it));
                        break;
                    }
                }

                return std::make_pair(std::move(result), std::next(it));
            }

            // Note: this function assumes that *--being == '$'
            std::pair<lexeme::argument, iterator> parse_argument(iterator begin, iterator end)
            {
                if(begin == end)
                    throw lexer_error("empty argument name");

                char_type first = *begin;
                if(detail::is_digit(first))
                {
                    iterator next = std::find_if_not(
                        std::next(begin), end,
                        &detail::is_digit
                    );
                    lexeme::argument::index_type idx;
                    auto conv = std::from_chars(
                        std::to_address(begin), std::to_address(end),
                        idx
                    );
                    assert(conv.ec == std::errc());
                    assert(conv.ptr == std::to_address(next));

                    return std::make_pair(idx, next);
                }
                else if(detail::is_identifier(first, true))
                {
                    iterator next = std::find_if_not(
                        std::next(begin), end,
                        [](char_type ch) { return detail::is_identifier(ch, false); }
                    );

                    return std::make_pair(string_type(begin, next), next);
                }
                else
                {
                    const char_type bad_name[2] = { first, '\0' };
                    throw invalid_argument_name(bad_name);
                }
            }

            [[nodiscard]]
            std::optional<lexeme::keyword> get_keyword(string_view_type str) noexcept
            {
                assert(!str.empty());

                if(str == "if")
                    return keyword_type::if_;
                else if(str == "else")
                    return keyword_type::else_;
                else if(str == "elif")
                    return keyword_type::elif;

                return std::nullopt;
            }

            [[nodiscard]]
            std::optional<operator_type> get_operator(string_view_type str)
            {
                using enum operator_type;

                if(str.length() == 1)
                {
                    switch(str[0])
                    {
                    case ':':
                        return colon;

                    case ',':
                        return comma;

                    case '.':
                        return dot;

                    case '(':
                        return parenthesis_l;
                    case ')':
                        return parenthesis_r;

                    case '[':
                        return bracket_l;
                    case ']':
                        return bracket_r;

                    case '!':
                        return not_;

                    case '<':
                        return less_than;
                    case '>':
                        return greater_than;
                    }
                }
                else if(str.length() == 2)
                {
                    using namespace std::literals;

                    if(str == "=="sv)
                        return equal;
                    else if(str == "!="sv)
                        return not_equal;
                    else if(str == "<="sv)
                        return less_equal;
                    else if(str == ">="sv)
                        return greater_equal;
                    else if(str == "&&"sv)
                        return and_;
                    else if(str == "||"sv)
                        return or_;
                }

                return std::nullopt;
            }
        };

        class script_parse_context
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;
            using string_view_type = std::basic_string_view<char_type>;

            void parse(string_view_type src);
        };
    }
}
