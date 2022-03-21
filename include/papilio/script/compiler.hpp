#pragma once

#include "exec.hpp"
#include "lexer.hpp"


namespace papilio::script
{
    namespace detailed
    {
        struct is_if
        {
            template <typename CharT>
            constexpr bool operator()(const basic_lexeme<CharT>& l) const noexcept
            {
                return
                    l.type() == lexeme_type::keyword &&
                    l.str() == basic_keywords<CharT>::if_();
            }
        };
        struct is_elif
        {
            template <typename CharT>
            constexpr bool operator()(const basic_lexeme<CharT>& l) const noexcept
            {
                return
                    l.type() == lexeme_type::keyword &&
                    l.str() == basic_keywords<CharT>::elif();
            }
        };
        struct is_else
        {
            template <typename CharT>
            constexpr bool operator()(const basic_lexeme<CharT>& l) const noexcept
            {
                return
                    l.type() == lexeme_type::keyword &&
                    l.str() == basic_keywords<CharT>::else_();
            }
        };
        struct is_end
        {
            template <typename CharT>
            constexpr bool operator()(const basic_lexeme<CharT>& l) const noexcept
            {
                return
                    l.type() == lexeme_type::keyword &&
                    l.str() == basic_keywords<CharT>::end();
            }
        };

        struct is_op_condition_end
        {
            template <typename CharT>
            constexpr bool operator()(const basic_lexeme<CharT>& l) const noexcept
            {
                return
                    l.type() == lexeme_type::operator_ &&
                    l.str() == basic_operators<CharT>::op_condition_end();
            }
        };

        template<typename... Ts>
        struct combined_or
        {
            template <typename CharT>
            constexpr bool operator()(const basic_lexeme<CharT>& l) const noexcept
            {
                return (Ts()(l) || ...);
            }
        };
    }

    template <typename CharT>
    class basic_compiler
    {
    public:
        typedef CharT char_type;
        typedef std::basic_string<char_type> string_type;
        typedef std::basic_string_view<char_type> string_view_type;
        typedef basic_keywords<char_type> keywords;
        typedef basic_operators<char_type> operators;
        typedef basic_lexer<char_type> lexer;
        typedef basic_context<char_type> context;
        typedef basic_lexeme<char_type> lexeme;

        template <typename Iterator>
        std::unique_ptr<typename context::script> compile(Iterator begin, Iterator end)
        {
            if(begin == end)
            {
                return std::make_unique<typename context::script_literal>(nullvar);
            }

            // First non-value lexeme
            auto non_value_begin = find_non_value(begin, end);

            if(non_value_begin == end)
            { // There are only literals or identifiers in the lexemes
                // Check syntax
                if(std::distance(begin, non_value_begin) > 1)
                { // Too many literals or identifiers
                    throw syntax_error();
                }

                switch(begin->type())
                {
                case lexeme_type::literal:
                    return compile_literal(*begin);
                case lexeme_type::identifier:
                    return compile_identifier(*begin);
                default:
                    throw syntax_error();
                }
            }

            switch(non_value_begin->type())
            {
            case lexeme_type::operator_:
                if(const lexeme& op = *non_value_begin; is_comparator(op.str()))
                {
                    auto op_it = non_value_begin;
                    ++non_value_begin;
                    non_value_begin = find_non_value(non_value_begin, end);
                    auto left = compile(begin, op_it);
                    ++op_it;
                    auto right = compile(op_it, non_value_begin);

                    return compile_comp(op, std::move(left), std::move(right));
                }
                break;

            case lexeme_type::keyword:
                if(const lexeme& kw = *non_value_begin; kw.str() == keywords::if_())
                {
                    return compile_if(non_value_begin, end).first;
                }
                break;
            }

            throw syntax_error();
        }
        std::unique_ptr<typename context::script> compile(std::span<const lexeme> lexemes)
        {
            return compile(lexemes.begin(), lexemes.end());
        }

     private:
        std::unique_ptr<typename context::script_literal> compile_literal(const lexeme& l)
        {
            assert(l.type() == lexeme_type::literal);
            string_view_type view = l.str();
            if(view[0] == char_type('"'))
            { // String literal
                assert(view.size() >= 2);
                // Strip quotes on each side
                view = string_view_type(view.begin() + 1, view.end() - 1);
                return std::make_unique<typename context::script_literal>(string_type(view));
            }
            else
            { // Numeric literal
                std::basic_stringstream<char_type> ss;
                ss << view;
                if(view.find(char_type('.')) == view.npos)
                { // Interger
                    int val;
                    ss >> val;
                    return std::make_unique<typename context::script_literal>(val);
                }
                else
                { // Floating point
                    float val;
                    ss >> val;
                    return std::make_unique<typename context::script_literal>(val);
                }
            }
        }
        std::unique_ptr<typename context::script_argument_any> compile_identifier(const lexeme& l)
        {
            assert(l.type() == lexeme_type::identifier);

            if(l.str().empty())
            { // Empty identifier
                throw syntax_error();
            }

            if(lexer::is_digit(l.str()[0]))
            { // Indexed argument
                std::basic_stringstream<char_type> ss;
                ss << l.str();
                std::size_t idx;
                ss >> idx;

                return std::make_unique<typename context::script_argument_any>(idx);
            }
            else
            { // Named argument
                return std::make_unique<typename context::script_argument_any>(l.str());
            }
        }
        std::unique_ptr<typename context::script> compile_comp(
            const lexeme& l,
            std::unique_ptr<typename context::script> left,
            std::unique_ptr<typename context::script> right
        ) {
            if(l.str() == operators::op_greater_than())
            {
                return make_comp<helper::greater>(
                    std::move(left), std::move(right)
                );
            }
            else if(l.str() == operators::op_greater_equal())
            {
                return make_comp<helper::greater_equal>(
                    std::move(left), std::move(right)
                );
            }
            else if(l.str() == operators::op_less_than())
            {
                return make_comp<helper::less>(
                    std::move(left), std::move(right)
                );
            }
            else if(l.str() == operators::op_less_than())
            {
                return make_comp<helper::less_equal>(
                    std::move(left), std::move(right)
                );
            }
            else if (l.str() == operators::op_equal())
            {
                return make_comp< helper::equal>(
                    std::move(left), std::move(right)
                );
            }
            else if (l.str() == operators::op_not_equal())
            {
                return make_comp<helper::not_equal>(
                    std::move(left), std::move(right)
                );
            }
            else
            {
                throw syntax_error();
            }
        }

        template <typename Iterator>
        auto compile_if(Iterator begin, Iterator end)->
             std::pair<std::unique_ptr<typename context::script_if>, Iterator>
        {
            assert(begin != end);
            assert(
                begin->type() == lexeme_type::keyword &&
                (begin->str() == keywords::if_() || begin->str() == keywords::elif() )
            );

            // Skip if/elif at the beginning
            ++begin;

            auto control_flow_end = std::find_if(
                begin, end,
                detailed::is_end()
            );

            auto if_ = std::make_unique<typename context::script_if>();
            auto cond_end = std::find_if(
                begin, control_flow_end,
                detailed::is_op_condition_end()
            );
            if(cond_end == control_flow_end)
            { // Missing ":" operator
                throw syntax_error();
            }
            if_->condition = compile(begin, cond_end);
            ++cond_end; // Skip the operator ":"
            auto true_block_end = std::find_if(
                cond_end, control_flow_end,
                detailed::combined_or<detailed::is_elif, detailed::is_else>()
            );
            if_->on_true = compile(cond_end, true_block_end);

            typename context::script_if* chain = if_.get();

            auto elif_begin = true_block_end;
            while(detailed::is_elif()(*elif_begin))
            {
                std::tie(chain->on_false, elif_begin) = compile_elif(elif_begin, control_flow_end);
                assert(dynamic_cast<typename context::script_if*>(chain->on_false.get()));
                chain = static_cast<typename context::script_if*>(chain->on_false.get());
            }

            auto else_begin = std::find_if(
                elif_begin, control_flow_end,
                detailed::is_else()
            );
            if(else_begin != control_flow_end)
            { // The "else" block exists
                ++else_begin; // Move to the operator ":"
                if(else_begin != control_flow_end &&
                    else_begin->type() == lexeme_type::operator_ && else_begin->str() == operators::op_condition_end())
                {
                    ++else_begin; // Skip the operator ":"
                    chain->on_false = compile(else_begin, control_flow_end);
                }
                else
                { // Missing operator ":" after "else"
                    throw syntax_error();
                }
            }

            return std::make_pair(std::move(if_), ++control_flow_end);
        }
        template <typename Iterator>
        auto compile_elif(Iterator begin, Iterator end)->
            std::pair<std::unique_ptr<typename context::script_if>, Iterator>
        {
            using namespace detailed;

            assert(is_elif()(*begin));

            // Skip "elif" at the beginning
            ++begin;

            auto block_end = std::find_if(
                begin, end,
                combined_or<is_end, is_else, is_elif>()
            );
            auto cond_end = std::find_if(
                begin, block_end,
                is_op_condition_end()
            );
            if(cond_end == block_end)
            { // Missing operator ":"
                throw syntax_error();
            }

            auto if_ = std::make_unique<typename context::script_if>();
            if_->condition = compile(begin, cond_end);

            ++cond_end;
            if_->on_true = compile(cond_end, block_end);

            return std::make_pair(std::move(if_), block_end);
        }

        // Auxiliary function
        // Find first non-value lexeme
        template <typename Iterator>
        static Iterator find_non_value(Iterator begin, Iterator end)
        {
            return std::find_if_not(
                begin, end,
                [](const lexeme& l)
                {
                    return
                        l.type() == lexeme_type::literal ||
                        l.type() == lexeme_type::identifier;
                }
            );
        }

        // Auxiliary function
        template <typename Comp>
        static std::unique_ptr<typename context::script> make_comp(
            std::unique_ptr<typename context::script> left,
            std::unique_ptr<typename context::script> right
        ) {
            using script_comp = context::template script_compare<Comp>;
            auto comp = std::make_unique<script_comp>();
            comp->left_operand = std::move(left);
            comp->right_operand = std::move(right);

            return comp;
        }
    };

    typedef basic_compiler<char> compiler;
    typedef basic_compiler<wchar_t> wcompiler;
    typedef basic_compiler<char16_t> u16compiler;
    typedef basic_compiler<char32_t> u32compiler;
    typedef basic_compiler<char8_t> u8compiler;
}
