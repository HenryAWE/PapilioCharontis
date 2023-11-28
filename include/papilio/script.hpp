#pragma once

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <stack>
#include <span>
#include <algorithm>
#include <memory>
#include <utility>
#include <cassert>
#include <charconv>
#include <optional>
#include <sstream>
#include "container.hpp"
#include "core.hpp"
#include "error.hpp"


namespace papilio::script
{
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

        [[nodiscard]]
        constexpr bool is_single_byte_operator_ch(char ch) noexcept
        {
            return
                ch == ':' ||
                ch == ',' ||
                ch == '.' ||
                ch == '[' ||
                ch == ']' ||
                ch == '!' ||
                ch == '<' ||
                ch == '>';
        }
        [[nodiscard]]
        constexpr bool is_operator_ch(char ch) noexcept
        {
            return
                is_single_byte_operator_ch(ch) ||
                ch == '=' ||
                ch == '|' ||
                ch == '&';
        }
    }

    enum class lexeme_type
    {
        argument = 1,
        identifier,
        constant,
        keyword,
        operator_,
        field   // replacement field
    };

    enum class keyword_type
    {
        if_ = 1,
        elif,
        else_
    };

    enum class operator_type
    {
        colon = 1,  // :
        comma,  // ,
        dot,    // .
        bracket_l,  // [
        bracket_r,   // ]
        not_,   // !
        equal, // ==
        not_equal,  // !=
        greater_than,   // >
        less_than,  // <
        greater_equal,  // >=
        less_equal, // <=
    };

    namespace detail
    {
        template <typename T>
        concept is_lexeme_helper = requires(T t)
        {
            { t.type }->std::same_as<const lexeme_type&>;
        };
    }

    template <typename T>
    struct is_lexeme : std::bool_constant<detail::is_lexeme_helper<T>> {};
    template <typename T>
    inline constexpr bool is_lexeme_v = is_lexeme<T>::value;

    enum class lexer_mode
    {
        standalone = 0,
        script_block,
        replacement_field
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
            using underlying_type = std::variant<index_type, utf::string_container>;

            static constexpr lexeme_type type = lexeme_type::argument;

            argument() = delete;
            argument(const argument&) = default;
            argument(argument&&) = default;
            constexpr argument(index_type idx) noexcept
                : m_arg(idx) {}
            argument(utf::string_container name)
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
            const utf::string_container& get_string() const noexcept
            {
                assert(holds<utf::string_container>());
                return *std::get_if<utf::string_container>(&m_arg);
            }

            [[nodiscard]]
            indexing_value to_indexing_value() const noexcept
            {
                auto visitor = [](auto&& v)
                {
                    return indexing_value(v);
                };
                return std::visit(visitor, m_arg);
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
            identifier(utf::string_container str)
                : m_str(std::move(str)) {}

            [[nodiscard]]
            const utf::string_container& get() const noexcept
            {
                return m_str;
            }

        private:
            utf::string_container m_str;
        };

        class constant
        {
        public:
            using int_type = std::int64_t;
            using float_type = long double;
            using underlying_type = std::variant<
                int_type,
                float_type,
                utf::string_container
            >;

            static constexpr lexeme_type type = lexeme_type::constant;

            constant() = delete;
            constant(const constant&) = default;
            constant(constant&&) noexcept = default;
            constant(int_type i)
                : m_const(i) {}
            constant(float_type f)
                : m_const(f) {}
            constant(utf::string_container str)
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
            const utf::string_container& get_string() const noexcept
            {
                assert(holds<utf::string_container>());
                return *std::get_if<utf::string_container>(&m_const);
            }

            [[nodiscard]]
            const underlying_type& to_underlying() const noexcept
            {
                return m_const;
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
            field(utf::string_container fmt_str)
                : m_fmt_str(std::move(fmt_str)) {}

            const utf::string_container& get() const noexcept
            {
                return m_fmt_str;
            }

        private:
            utf::string_container m_fmt_str;
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
        template <typename Lexeme> requires is_lexeme_v<Lexeme>
        lexeme(Lexeme lex)
            : m_store(std::move(lex)) {}

        lexeme_type type() const noexcept
        {
            return std::visit(
                [](auto&& v) { return v.type; },
                m_store
            );
        }

        template <typename T> requires is_lexeme_v<T>
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_store);
        }
        template <typename T> requires is_lexeme_v<T>
        const T& as() const noexcept
        {
            assert(holds<T>());
            return *std::get_if<T>(&m_store);
        }

    private:
        lexeme_store m_store;
    };

    // NOTE: This function assumes that *--being == '['
    // and it will find the first mismatched right bracket ']'
    template <typename Iterator>
    Iterator find_script_end(Iterator begin, Iterator end)
    {
        auto pred = [counter = std::size_t(0)](const auto& ch) mutable
        {
            if(ch == '[')
                ++counter;
            if(ch == ']')
            {
                if(counter == 0)
                    return true;
                --counter;
            }

            return false;
        };

        return std::find_if(begin, end, pred);
    }
    // NOTE: This function assumes that *--being == ']'
    // and it will find the first mismatched left bracket '['
    template <typename ReverseIterator>
    ReverseIterator rfind_script_begin(ReverseIterator begin, ReverseIterator end)
    {
        auto pred = [counter = std::size_t(0)](const auto& ch) mutable
        {
            if(ch == ']')
                ++counter;
            if(ch == '[')
            {
                if(counter == 0)
                    return true;
                --counter;
            }

            return false;
        };

        return std::find_if(begin, end, pred);
    }

    // NOTE: This function assumes that *--being == '{'
    // and it will find the first mismatched right brace '}'
    template <typename Iterator>
    Iterator find_field_end(Iterator begin, Iterator end)
    {
        auto pred = [counter = std::size_t(0)](const auto& ch) mutable
        {
            if(ch == '{')
                ++counter;
            if(ch == '}')
            {
                if(counter == 0)
                    return true;
                --counter;
            }

            return false;
        };

        return std::find_if(begin, end, pred);
    }
    // NOTE: This function assumes that *--being == '}'
    // and it will find the first mismatched left brace '{'
    template <typename ReverseIterator>
    ReverseIterator rfind_field_begin(ReverseIterator begin, ReverseIterator end)
    {
        std::size_t counter = 0;

        // std::find_if will cause compile errors on GCC
        for(auto it = begin; it != end; ++it)
        {
            char32_t ch = *it;
            if(ch == U'}')
                ++counter;
            if(ch == U'{')
            {
                if(counter == 0)
                    return it;
                --counter;
            }
        }
        return end;
    }

    class lexer
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using const_iterator =  string_view_type::const_iterator;
        using iterator = const_iterator;
        using lexeme_storage = small_vector<lexeme, 8>;

        struct parse_result
        {
            std::size_t parsed_char = 0; // parsed characters count
            bool default_arg_idx_used = false;
        };

        // parse string and return parse result (see above)
        // mode: if mode is script block or replacement field,
        // this function will assume that *(src.data() - 1) == '[' or '{', respectively
        // default_arg_idx: this argument is used for replacement field mode for default index
        parse_result parse(
            string_view_type src,
            lexer_mode mode = lexer_mode::standalone,
            std::optional<std::size_t> default_arg_idx = std::nullopt
        );

        std::span<const lexeme> lexemes() const& noexcept
        {
            return m_lexemes;
        }

        void clear() noexcept
        {
            m_lexemes.clear();
        }

    private:
        std::vector<lexeme> m_lexemes;

        template <typename Lexeme, typename... Args> requires is_lexeme_v<Lexeme>
        void push_lexeme(Args&&... args)
        {
            m_lexemes.emplace_back(Lexeme(std::forward<Args>(args)...));
        }

        iterator consume_whitespace(iterator begin, iterator end)
        {
            return std::find_if_not(begin, end, &detail::is_space);
        }

        auto parse_number(iterator begin, iterator end)->std::pair<lexeme::constant, iterator>;
        // Note: this function assumes that *--begin == '\''
        auto parse_string(iterator begin, iterator end)->std::pair<string_type, iterator>;

        // Note: this function assumes that *--being == '$'
        auto parse_argument(iterator begin, iterator end)->std::pair<lexeme::argument, iterator>;

        [[nodiscard]]
        std::optional<lexeme::keyword> get_keyword(string_view_type str) noexcept;

        [[nodiscard]]
        static std::pair<operator_type, std::size_t> get_operator(string_view_type str) noexcept;
    };

    class executor
    {
    public:
        using char_type = variable::char_type;
        using int_type = variable::int_type;
        using float_type = variable::float_type;
        using string_type = std::basic_string<char_type>;
        using stack_type = std::stack<
            variable,
            small_vector<variable, 4>
        >;

        class context
        {
        public:
            context() = delete;
            context(const dynamic_format_args& arg_store)
                : m_arg_store(arg_store) {}

            [[nodiscard]]
            stack_type& get_stack() noexcept
            {
                return m_var_stack;
            }
            [[nodiscard]]
            const stack_type& get_stack() const noexcept
            {
                return m_var_stack;
            }

            [[nodiscard]]
            const dynamic_format_args& get_store() const noexcept
            {
                return m_arg_store;
            }

            [[nodiscard]]
            bool empty() const noexcept
            {
                return m_var_stack.empty();
            }

            const variable& top() const
            {
                assert(!m_var_stack.empty());
                return m_var_stack.top();
            }

            void pop() noexcept
            {
                assert(!m_var_stack.empty());
                m_var_stack.pop();
            }
            variable copy_and_pop()
            {
                assert(!m_var_stack.empty());
                variable tmp = std::move(m_var_stack.top());
                m_var_stack.pop();
                return tmp;
            }

            void push(const variable& var)
            {
                m_var_stack.push(var);
            }

            string_type get_result()
            {
                if(empty())
                    return string_type();
                else
                    return top().as<string_type>();
            }

        private:
            stack_type m_var_stack;
            dynamic_format_args m_arg_store;
        };

        class base
        {
        public:
            virtual void execute(context& ctx) const = 0;
        };

        template <typename T>
        class constant final : public base
        {
        public:
            using value_type = T;

            constant() = default;
            constant(const constant&) = default;
            constant(constant&&) = default;
            constant(const value_type& val)
                : m_val(val) {}
            constant(value_type&& val)
                : m_val(std::move(val)) {}
            template <typename... Args>
            constant(std::in_place_t, Args&&... args)
                : m_val(std::forward<Args>(args)...) {}

            void execute(context& ctx) const override
            {
                ctx.push(m_val);
            }

        private:
            value_type m_val;
        };

        class selection final : public base
        {
        public:
            selection() = delete;
            selection(const selection&) = delete;
            selection(selection&&) = delete;
            selection(
                std::unique_ptr<base> cond,
                std::unique_ptr<base> on_true,
                std::unique_ptr<base> on_false = nullptr
            ) : m_cond(std::move(cond)), m_on_true(std::move(on_true)), m_on_false(std::move(on_false)) {}

            void execute(context& ctx) const override
            {
                assert(m_cond);
                m_cond->execute(ctx);
                bool result = ctx.copy_and_pop().as<bool>();
                if(result)
                {
                    m_on_true->execute(ctx);
                }
                else if(m_on_false)
                { // this branch could be empty
                    m_on_false->execute(ctx);
                }
            }

        private:
            std::unique_ptr<base> m_cond;
            std::unique_ptr<base> m_on_true;
            std::unique_ptr<base> m_on_false;
        };

        class argument final : public base
        {
        public:
            using member_type = format_arg_access::member_type;
            using member_storage = format_arg_access::member_storage;

            argument() = delete;
            argument(indexing_value arg_id)
                : m_arg_id(std::move(arg_id)), m_access() {}
            argument(indexing_value arg_id, member_storage members)
                : m_arg_id(std::move(arg_id)), m_access(std::move(members)) {}
            argument(indexing_value arg_id, format_arg_access access)
                : m_arg_id(std::move(arg_id)), m_access(std::move(access)) {}

            void execute(context& ctx) const override
            {
                format_arg arg = ctx.get_store().get(m_arg_id);
                if(!m_access.empty())
                    arg = m_access.access(arg);

                auto var = arg.as_variable();
                ctx.push(std::move(var));
            }

        private:
            indexing_value m_arg_id;
            format_arg_access m_access;
        };

        template <typename Compare>
        class comparator final : public base
        {
        public:
            comparator(
                std::unique_ptr<base> lhs,
                std::unique_ptr<base> rhs
            ) : m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_comp() {}

            virtual void execute(context& ctx) const override
            {
                m_lhs->execute(ctx);
                m_rhs->execute(ctx);
                auto rhs_result = ctx.copy_and_pop();
                auto lhs_result = ctx.copy_and_pop();
                    
                bool result = m_comp(lhs_result, rhs_result);
                ctx.push(result);
            }

        private:
            std::unique_ptr<base> m_lhs;
            std::unique_ptr<base> m_rhs;
            Compare m_comp;
        };

        class logical_not : public base
        {
        public:
            logical_not() = delete;
            logical_not(std::unique_ptr<base> input)
                : m_input(std::move(input)) {}

            void execute(context& ctx) const override
            {
                m_input->execute(ctx);
                bool input = ctx.copy_and_pop().as<bool>();
                ctx.push(!input);
            }

        private:
            std::unique_ptr<base> m_input;
        };

        executor() = default;
        executor(const executor&) = delete;
        executor(executor&&) noexcept = default;
        executor(std::unique_ptr<base> ex)
            : m_ex(std::move(ex)) {}
        template <std::derived_from<base> T, typename... Args>
        executor(std::in_place_type_t<T>, Args&&... args)
            : executor(std::make_unique<T>(std::forward<Args>(args)...)) {}

        [[nodiscard]]
        bool empty() const noexcept
        {
            return m_ex == nullptr;
        }
        [[nodiscard]]
        operator bool() const noexcept
        {
            return !empty();
        }

        void operator()(context& ctx) const
        {
            assert(!empty());
            m_ex->execute(ctx);
        }

        void reset(std::unique_ptr<base> ex = nullptr)
        {
            m_ex.swap(ex);
        }
        template <std::derived_from<base> T, typename... Args>
        void emplace(Args&&... args)
        {
            m_ex = std::make_unique<T>(std::forward<Args>(args)...);
        }

        std::unique_ptr<base> release() noexcept
        {
            return std::move(m_ex);
        }

    private:
        std::unique_ptr<base> m_ex;
    };

    namespace detail
    {
        template <typename T>
        concept not_foramt_arg_store = !format_args<T>;
    }

    class interpreter
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        string_type run(string_view_type src, const dynamic_format_args& args);
        template <detail::not_foramt_arg_store... Args>
        string_type run(string_view_type src, Args&&... args)
        {
            return run(
                src,
                dynamic_format_args(papilio::make_format_args(std::forward<Args>(args)...))
            );
        }

        executor compile(string_view_type src);
        executor compile(std::span<const lexeme> lexemes);

        std::pair<indexing_value, format_arg_access> access(
            string_view_type arg,
            std::optional<std::size_t> default_arg_id = std::nullopt
        );
        std::pair<indexing_value, format_arg_access> access(std::span<const lexeme> lexemes);

    private:
        executor to_executor(std::span<const lexeme> lexemes);
        std::pair<indexing_value, format_arg_access> to_access(std::span<const lexeme> lexemes);
    };
}
