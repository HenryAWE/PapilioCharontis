#include <papilio/script.hpp>
#include <algorithm>
#include <papilio/core.hpp>
#include <papilio/detail/compat.hpp>


namespace papilio::script
{
    lexer::parse_result lexer::parse(
        string_view_type src,
        lexer_mode mode,
        std::optional<std::size_t> default_arg_idx
    ) {
        parse_result result;
        std::size_t bracket_counter = 0;
        bool parsing_condition = false;

        if(src.empty() && mode == lexer_mode::replacement_field)
        {
            result.default_arg_idx_used = true;
            // insert default argument
            if(default_arg_idx.has_value())
                push_lexeme<lexeme::argument>(*default_arg_idx);
            else
                throw lexer_error("can not deduce default argument here");
        }

        auto it = src.begin();
        for(; it != src.end();)
        {
            char_type ch = *it;

            if(detail::is_space(ch))
            {
                it = consume_whitespace(std::next(it), src.end());
            }
            else if(ch == '$')
            {
                auto parsed = parse_argument(
                    std::next(it), src.end()
                );
                push_lexeme<lexeme::argument>(std::move(parsed.first));
                it = parsed.second;
            }
            else if(ch == '\'')
            {
                auto parsed = parse_string(
                    std::next(it), src.end()
                );
                push_lexeme<lexeme::constant>(std::move(parsed.first));
                it = parsed.second;
            }
            else if(ch == '{')
            {
                iterator next = std::find(
                    it, src.end(),
                    '}'
                );
                if(next == src.end())
                {
                    throw lexer_error("lexer error");
                }

                push_lexeme<lexeme::field>(
                    string_type(std::next(it), next)
                );

                it = std::next(next);
            }
            else if(detail::is_digit(ch) || ch == '-')
            {
                // '$' is optional in the replacement field mode
                if(m_lexemes.size() == 0 && mode == lexer_mode::replacement_field)
                {
                    auto parsed = parse_argument(
                        it, src.end()
                    );
                    push_lexeme<lexeme::argument>(std::move(parsed.first));
                    it = parsed.second;
                    continue;
                }
                auto parsed = parse_number(it, src.end());
                push_lexeme<lexeme::constant>(std::move(parsed.first));
                it = parsed.second;
            }
            else if(detail::is_identifier(ch, true))
            {
                // '$' is optional in the replacement field mode
                if(m_lexemes.size() == 0 && mode == lexer_mode::replacement_field)
                {
                    auto parsed = parse_argument(
                        it, src.end()
                    );
                    push_lexeme<lexeme::argument>(std::move(parsed.first));
                    it = parsed.second;
                    continue;
                }
                auto next = std::find_if_not(
                    std::next(it), src.end(),
                    [](char_type ch) { return detail::is_identifier(ch, false); }
                );
                string_view_type sv(it, next);
                auto kw = get_keyword(sv);
                if(kw.has_value())
                {
                    parsing_condition = true;
                    push_lexeme<lexeme::keyword>(*kw);
                }
                else
                {
                    push_lexeme<lexeme::identifier>(sv);
                }

                it = next;
            }
            else if(detail::is_operator_ch(ch))
            {
                iterator next = std::find_if_not(
                    std::next(it), src.end(),
                    detail::is_operator_ch
                );

                string_view_type sv(it, next);
                std::size_t parsed_op_ch = 0;
                while(!sv.empty())
                {
                    auto op = get_operator(sv);
                    if(op.second == 0)
                        break;
                    if(op.first == operator_type::bracket_l)
                    {
                        ++bracket_counter;
                        if(m_lexemes.size() == 0 && mode == lexer_mode::replacement_field )
                        {
                            result.default_arg_idx_used = true;
                            // insert default argument
                            if(default_arg_idx.has_value())
                                push_lexeme<lexeme::argument>(*default_arg_idx);
                            else
                                throw lexer_error("can not deduce default argument here");
                        }
                    }
                    else if(op.first == operator_type::bracket_r)
                    {
                        // this right bracket indicates the end of script block
                        if(bracket_counter == 0)
                        {
                            // parsed characters don't include the ending bracket
                            if(mode == lexer_mode::script_block)
                            {
                                result.parsed_char = std::distance(src.begin(), it);
                                return result;
                            }
                        }
                        else
                        {
                            // normal right bracket for indexing
                            --bracket_counter;
                        }
                    }
                    else if(op.first == operator_type::colon && mode == lexer_mode::replacement_field)
                    {
                        if(!parsing_condition && bracket_counter == 0)
                        {
                            if(m_lexemes.size() == 0)
                            {
                                result.default_arg_idx_used = true;
                                // insert default argument
                                if(default_arg_idx.has_value())
                                    push_lexeme<lexeme::argument>(*default_arg_idx);
                                else
                                    throw lexer_error("can not deduce default argument here");
                            }
                            result.parsed_char = std::distance(src.begin(), it) + parsed_op_ch;
                            return result;
                        }
                    }
                    else if(
                        op.first == operator_type::dot &&
                        m_lexemes.size() == 0
                        && mode == lexer_mode::replacement_field
                    ) {
                        result.default_arg_idx_used = true;
                        // insert default argument
                        if(default_arg_idx.has_value())
                            push_lexeme<lexeme::argument>(*default_arg_idx);
                        else
                            throw lexer_error("can not deduce default argument here");
                    }

                    sv = sv.substr(op.second);
                    parsed_op_ch += op.second;

                    push_lexeme<lexeme::operator_>(op.first);
                }

                if(!sv.empty())
                {
                    throw lexer_error("unknown operator \"" + std::string(sv) + '\"');
                }

                it = next;
            }
            else if(ch == '}' && mode == lexer_mode::replacement_field)
            {
                if(m_lexemes.size() == 0)
                {
                    result.default_arg_idx_used = true;
                    // insert default argument
                    if(default_arg_idx.has_value())
                        push_lexeme<lexeme::argument>(*default_arg_idx);
                    else
                        throw lexer_error("can not deduce default argument here");
                }
                result.parsed_char = std::distance(src.begin(), it);
                return result;
            }
            else if(ch <= '~')
            {
                iterator next = std::find_if(
                    std::next(it), src.end(),
                    [](char_type ch)
                    {
                        return
                            detail::is_space(ch) ||
                            detail::is_alpha(ch) ||
                            detail::is_digit(ch);
                    }
                );
                    
                // TODO: Better error information
                (void)next;
                throw lexer_error("lexer error");
            }
            else
            {
                throw lexer_error("lexer error");
            }
        }

        result.parsed_char = std::distance(src.begin(), it);
        return result;
    }

    auto lexer::parse_number(iterator begin, iterator end)->std::pair<lexeme::constant, iterator>
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
                break;
            [[unlikely]] case 8:
                if(!('0' <= ch && ch <= '7'))
                    goto end_loop;
                break;
            [[likely]] case 10:
                if(!detail::is_digit(ch))
                    goto end_loop;
                break;
            case 16:
                if(!detail::is_xdigit(ch))
                    goto end_loop;
                break;

            default:
                PAPILIO_UNREACHABLE();
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
    auto lexer::parse_string(iterator begin, iterator end)->std::pair<string_type, iterator>
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

        throw std::runtime_error("missing quote (\"'\")");
    }

    auto lexer::parse_argument(iterator begin, iterator end)->std::pair<lexeme::argument, iterator>
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

            return std::make_pair(utf::string_container(begin, next), next);
        }
        else
        {
            const char_type bad_name[2] = { first, '\0' };
            throw invalid_argument_name(bad_name);
        }
    }

    std::optional<lexeme::keyword> lexer::get_keyword(string_view_type str) noexcept
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

    std::pair<operator_type, std::size_t> lexer::get_operator(string_view_type str) noexcept
    {
        using enum operator_type;
        using namespace std::literals;

        if(detail::is_single_byte_operator_ch(str[0]))
        {
            operator_type op;
            switch(str[0])
            {
            default:
                // jump to the end of this function to report an error
                goto unknown_operator;

            case ':':
                op = colon;
                break;
            case ',':
                op = comma;
                break;
            case '.':
                op = dot;
                break;
            case '[':
                op = bracket_l;
                break;
            case ']':
                op = bracket_r;
                break;
            case '!':
                // avoid ambiguity
                if(str.substr(0, 2) == "!="sv)
                    return std::make_pair(not_equal, 2);
                op = not_;
                break;
            case '<':
                op = less_than;
                break;
            case '>':
                op = greater_than;
                break;
            }

            return std::make_pair(op, 1);
        }
        else
        {
            // not equal (!=) is handled in the above code to avoid ambiguity
            str = str.substr(0, 2);
            operator_type op;
            if(str == "=="sv)
                op = equal;
            else if(str == "<="sv)
                op = less_equal;
            else if(str == ">="sv)
                op = greater_equal;
            else
                goto unknown_operator;

            return std::make_pair(op, 2);
        }

    unknown_operator:
        return std::make_pair(static_cast<operator_type>(0), 0);
    }

    interpreter::string_type interpreter::run(string_view_type src, const dynamic_format_args& args)
    {
        auto ex = compile(src);

        executor::context ctx(args);
        ex(ctx);

        return ctx.get_result();
    }
    executor interpreter::compile(string_view_type src)
    {
        lexer l;
        l.parse(src);
        return compile(l.lexemes());
    }
    executor interpreter::compile(std::span<const lexeme> lexemes)
    {
        return to_executor(lexemes);
    }

    std::pair<indexing_value, chained_access> interpreter::access(
        string_view_type arg,
        std::optional<std::size_t> default_arg_id
    ) {
        lexer l;
        l.parse(arg, lexer_mode::replacement_field, default_arg_id);
        return access(l.lexemes());
    }
    std::pair<indexing_value, chained_access> interpreter::access(std::span<const lexeme> lexemes)
    {
        return to_access(lexemes);
    }

    namespace detail
    {
        class executor_builder
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;

            using span_type = std::span<const lexeme>;
            using iterator = span_type::iterator;
            using reverse_iterator = span_type::reverse_iterator;
            using pointer = std::span<const lexeme>::const_pointer;

            // this function assumes that *--begin == if_ or elif
            std::pair<std::unique_ptr<executor::base>, iterator> build_selection(
                iterator begin, iterator end
            ) {
                auto cond = build_condition(begin, end);

                auto next_branch_pred = [](const lexeme& l)->bool
                {
                    if(l.type() == lexeme_type::keyword)
                    {
                        auto& kw = l.as<lexeme::keyword>();
                        return kw.get() == keyword_type::elif || kw.get() == keyword_type::else_;
                    }
                    else
                        return false;
                };
                auto next_branch = std::find_if(
                    cond.second, end, next_branch_pred
                );

                iterator next_it;
                auto string_build_result = build_string_expression(cond.second, next_branch);

                std::pair<std::unique_ptr<executor::base>, iterator> next_branch_build_result;
                if(next_branch != end)
                {
                    auto it = next_branch;
                    auto& kw = it->as<lexeme::keyword>();
                    if(kw.get() == keyword_type::else_)
                    {
                        // skip "else" and ":"
                        ++it;
                        if(
                            it == end ||
                            !(it->type() == lexeme_type::operator_ && it->as<lexeme::operator_>().get() == operator_type::colon)
                            ) {
                            raise_syntax_error("missing colon (':') after \"else\"");
                        }
                        ++it;

                        auto else_result = build_string_expression(it, end);
                        next_branch_build_result = std::move(else_result);
                    }
                    else if(kw.get() == keyword_type::elif)
                    {
                        auto elif_result = build_selection(std::next(it), end);
                        next_branch_build_result = std::move(elif_result);
                    }
                    else
                    {
                        raise_syntax_error("unknown error");
                    }

                    next_it = next_branch_build_result.second;
                }
                else
                {
                    next_it = next_branch;
                }

                auto ex = std::make_unique<executor::selection>(
                    std::move(cond.first),
                    std::move(string_build_result.first),
                    std::move(next_branch_build_result.first)
                    );
                return std::make_pair(std::move(ex), next_it);
            }

            // this function assumes that *--begin == if_ or elif
            std::pair<std::unique_ptr<executor::base>, iterator> build_condition(
                iterator begin, iterator end
            ) {
                auto pred = [counter = std::size_t(0), this](const lexeme& l) mutable->bool
                {
                    if(l.type() != lexeme_type::operator_)
                        return false;
                    auto op = l.as<lexeme::operator_>().get();
                    if(op == operator_type::bracket_l)
                        ++counter;
                    else if(op == operator_type::bracket_r)
                    {
                        if(counter == 0)
                        {
                            raise_syntax_error("too many right brackets (']')");
                        }
                        --counter;
                    }
                    else if(op == operator_type::colon && counter == 0)
                        return true;
                    return false;
                };
                iterator colon = std::find_if(
                    begin, end, pred
                );
                if(colon == end)
                {
                    throw std::invalid_argument("missing colon");
                }
                    
                auto build_result = build_bool_expression(begin, colon);
                assert(build_result.second == colon);
                return std::make_pair(
                    std::move(build_result.first),
                    std::next(colon)
                );
            }

            std::pair<std::unique_ptr<executor::base>, iterator> build_bool_expression(
                iterator begin, iterator end
            ) {
                std::unique_ptr<executor::base> result;

                const std::size_t count = std::distance(begin, end);
                if(
                    begin->type() == lexeme_type::operator_ &&
                    begin->as<lexeme::operator_>().get() == operator_type::not_
                ) {
                    auto input = build_input(std::next(begin), end);
                    if(input.second != end)
                    {
                        raise_syntax_error("invalid Boolean expression");
                    }

                    return std::make_pair(
                        std::make_unique<executor::logical_not>(std::move(input.first)),
                        end
                    );
                }
                else
                {
                    // TODO: bug fix
                    auto comp_pred = [](const lexeme& l)->bool
                    {
                        if(l.type() != lexeme_type::operator_)
                            return false;

                        using T = std::underlying_type_t<operator_type>;
                        T val = static_cast<T>(l.as<lexeme::operator_>().get());
                        return
                            static_cast<T>(operator_type::equal) <= val &&
                            val <= static_cast<T>(operator_type::less_equal);
                    };

                    iterator comp_it = std::find_if(
                        begin, end,
                        comp_pred
                    );
                    if(comp_it == end)
                    {
                        if(begin->type() == lexeme_type::argument)
                        {
                            auto build_result = build_argument(begin, end);
                            if(build_result.second != end)
                            {
                                raise_syntax_error("invalid Boolean expression");
                            }

                            return std::move(build_result);
                        }
                        else if(begin->type() == lexeme_type::constant)
                        {
                            auto build_result = build_constant(begin, end);
                            if(build_result.second != end)
                            {
                                raise_syntax_error("invalid Boolean expression");
                            }

                            return std::move(build_result);
                        }
                    }
                    else
                    {
                        auto lhs = build_input(begin, comp_it);
                        auto rhs = build_input(std::next(comp_it), end);

                        auto ex = get_comparator(
                            comp_it->as<lexeme::operator_>().get(),
                            std::move(lhs.first), std::move(rhs.first)
                        );

                        return std::make_pair(std::move(ex), rhs.second);
                    }
                }

                raise_syntax_error("invalid Boolean expression");
            }

            std::pair<std::unique_ptr<executor::base>, iterator> build_string_expression(
                iterator begin, iterator end
            ) {
                const std::size_t count = std::distance(begin, end);
                auto& l = *begin;

                if(count == 1)
                {
                    if(l.type() == lexeme_type::constant)
                    {
                        auto& c = l.as<lexeme::constant>();
                            
                        if(!c.holds<utf::string_container>())
                        {
                            raise_syntax_error("result type is not string");
                        }

                        return std::make_pair(
                            std::make_unique<executor::constant<utf::string_container>>(c.get_string()),
                            end
                        );
                    }
                        
                }
                if(l.type() == lexeme_type::argument)
                {
                    auto result = build_argument(begin, end);
                    return std::move(result);
                }

                raise_syntax_error("failed to build string expression");
            }

            std::pair<std::unique_ptr<executor::base>, iterator> build_input(
                iterator begin, iterator end
            ) {
                if(begin->type() == lexeme_type::constant)
                {
                    return build_constant(begin, end);
                }
                else if(begin->type() == lexeme_type::argument)
                {
                    return build_argument(begin, end);
                }

                raise_syntax_error("invalid input");
            }
            std::pair<std::unique_ptr<executor::base>, iterator> build_constant(
                iterator begin, iterator end
            ) {
                assert(begin->type() == lexeme_type::constant);
                auto& c = begin->as<lexeme::constant>();

                auto visitor = [](auto&& v)->std::unique_ptr<executor::base>
                {
                    using T = std::remove_cvref_t<decltype(v)>;

                    return std::make_unique<executor::constant<T>>(v);
                };
                auto ex = std::visit(visitor, c.to_underlying());

                return std::make_pair(std::move(ex), std::next(begin));
            }
            std::pair<std::unique_ptr<executor::argument>, iterator> build_argument(
                iterator begin, iterator end
            ) {
                auto [access, next_it] = build_access(begin, end);
                auto ex = std::make_unique<executor::argument>(
                    std::move(access.first),
                    std::move(access.second)
                );
                return std::make_pair(std::move(ex), next_it);
            }

            std::pair<std::pair<indexing_value, chained_access>, iterator> build_access(
                iterator begin, iterator end
            ) {
                assert(begin != end);
                assert(begin->type() == lexeme_type::argument);

                auto& a = begin->as<lexeme::argument>();
                chained_access::container_type members;

                auto it = std::next(begin);
                for(; it != end;)
                {
                    if(it->type() == lexeme_type::operator_)
                    {
                        auto& op = it->as<lexeme::operator_>();
                        if(op.get() == operator_type::bracket_l)
                        {
                            auto [idx, next_it] = build_index(std::next(it), end);
                            members.push_back(std::move(idx));
                            it = next_it;
                        }
                        else if(op.get() == operator_type::dot)
                        {
                            iterator identifier_it = std::next(it);
                            if(identifier_it == end || identifier_it->type() != lexeme_type::identifier)
                            {
                                raise_syntax_error("invalid member");
                            }

                            auto& id = identifier_it->as<lexeme::identifier>();
                            members.push_back(attribute_name(id.get()));
                            it = std::next(identifier_it);
                        }
                        else
                            break;
                    }
                    else
                        break;
                }

                return std::make_pair(
                    std::make_pair(a.to_indexing_value(), std::move(members)),
                    it
                );
            }

            // this function assumes that *--begin == '[' and begin != end
            std::pair<indexing_value, iterator> build_index(
                iterator begin, iterator end
            ) {
                assert(begin != end);

                iterator slice_op = end;
                iterator right_bracket = begin;
                for(; right_bracket != end; ++right_bracket)
                {
                    const lexeme& l = *right_bracket;
                    if(l.type() == lexeme_type::operator_)
                    {
                        auto& op = l.as<lexeme::operator_>();
                        if(op.get() == operator_type::colon)
                        {
                            if(slice_op != end)
                                raise_syntax_error("too many colons for a slice");
                            slice_op = right_bracket;
                        }
                        if(op.get() == operator_type::bracket_r)
                            break;
                    }
                }
                if(right_bracket == end)
                {
                    raise_syntax_error("missing right bracket (']')");
                }

                if(slice_op == end)
                {
                    if(begin->type() == lexeme_type::constant)
                    {
                        auto& c = begin->as<lexeme::constant>();
                        if(c.holds<lexeme::constant::float_type>())
                        {
                            raise_syntax_error("the type of index cannot be float");
                        }
                        else
                        {
                            if(std::distance(begin, right_bracket) > 1)
                            {
                                raise_syntax_error("too many values for index");
                            }

                            if(c.holds<utf::string_container>())
                                return std::make_pair(c.get_string(), std::next(right_bracket));
                            else
                                return std::make_pair(c.get_int(), std::next(right_bracket));
                        }
                    }
                }
                else
                {
                    return std::make_pair(
                        handle_slice_expression(begin, right_bracket, slice_op),
                        std::next(right_bracket)
                    );
                }

                raise_syntax_error("invalid index");
            }

            [[noreturn]]
            void raise_syntax_error(const std::string& msg)
            {
                throw std::runtime_error(msg);
            }

        private:
            slice handle_slice_expression(
                iterator begin, iterator end,
                iterator slice_op
            ) {
                assert(begin != end);

                std::pair<slice::index_type, slice::index_type> result;;

                std::size_t slice_begin_lexeme_count = std::distance(begin, slice_op);
                if(slice_begin_lexeme_count == 0)
                {
                    result.first = 0;
                }
                else if(slice_begin_lexeme_count == 1)
                {
                    if(begin->type() == lexeme_type::constant)
                    {
                        auto& c = begin->as<lexeme::constant>();
                        if(!c.holds<lexeme::constant::int_type>())
                        {
                            raise_syntax_error("value for slicing must be integer");
                        }

                        result.first = c.get_int();
                    }
                    else
                    {
                        raise_syntax_error("invalid index value");
                    }
                }
                else
                {
                    raise_syntax_error("too many arguments for slicing");
                }

                iterator slice_end_it = std::next(slice_op);
                std::size_t slice_end_lexeme_count = std::distance(slice_end_it, end);
                if(slice_end_lexeme_count == 0)
                {
                    result.second = slice::npos;
                }
                else if(slice_end_lexeme_count == 1)
                {
                    if(slice_end_it->type() == lexeme_type::constant)
                    {
                        auto& c = slice_end_it->as<lexeme::constant>();
                        if(!c.holds<lexeme::constant::int_type>())
                        {
                            raise_syntax_error("value for slicing must be integer");
                        }

                        result.second = c.get_int();
                    }
                    else
                    {
                        raise_syntax_error("invalid index value");
                    }
                }
                else
                {
                    raise_syntax_error("too many arguments for slicing");
                }

                return slice(result.first, result.second);
            }

            std::unique_ptr<executor::base> get_comparator(
                operator_type op,
                std::unique_ptr<executor::base> lhs,
                std::unique_ptr<executor::base> rhs
            ) {
#define PAPILIO_MAKE_COMPARATOR(comp) std::make_unique<executor::comparator<comp>>(std::move(lhs), std::move(rhs))

                switch(op)
                {
                    using enum operator_type;
                case equal:
                    return PAPILIO_MAKE_COMPARATOR(std::equal_to<>);
                case not_equal:
                    return PAPILIO_MAKE_COMPARATOR(std::not_equal_to<>);;
                case greater_than:
                    return PAPILIO_MAKE_COMPARATOR(std::greater<>);;
                case less_than:
                    return PAPILIO_MAKE_COMPARATOR(std::less<>);;
                case greater_equal:
                    return PAPILIO_MAKE_COMPARATOR(std::greater_equal<>);;
                case less_equal:
                    return PAPILIO_MAKE_COMPARATOR(std::less_equal<>);;
                }

                raise_syntax_error("invalid comparator");
            }
        };
    }

    executor interpreter::to_executor(std::span<const lexeme> lexemes)
    {
        detail::executor_builder builder;
        std::unique_ptr<executor::base> result;

        auto it = lexemes.begin();
        for(; it != lexemes.end();)
        {
            if(it->type() == lexeme_type::keyword)
            {
                if(it->as<lexeme::keyword>().get() == keyword_type::if_)
                {
                    std::tie(result, it) = builder.build_selection(std::next(it), lexemes.end());
                    break;
                }
            }
            else if(it->type() == lexeme_type::argument)
            {
                std::tie(result, it) = builder.build_argument(it, lexemes.end());
                break;
            }
            else if(it->type() == lexeme_type::constant)
            {
                std::tie(result, it) = builder.build_string_expression(it, lexemes.end());
                break;
            }
            else
            {
                throw std::runtime_error("syntax error");
            }
        }

        if(it != lexemes.end())
        {
            throw std::runtime_error("syntax error");
        }
        return std::move(result);
    }

    std::pair<indexing_value, chained_access> interpreter::to_access(std::span<const lexeme> lexemes)
    {
        detail::executor_builder builder;

        assert(!lexemes.empty());
        if(lexemes[0].type() != lexeme_type::argument)
            builder.raise_syntax_error("invalid access");

        auto [result, it] = builder.build_access(lexemes.begin(), lexemes.end());
        assert(it == lexemes.end());

        return std::move(result);
    }
}
