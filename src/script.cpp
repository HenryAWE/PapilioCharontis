#include <papilio/script.hpp>
#include <algorithm>
#include <papilio/core.hpp>


namespace papilio
{
    namespace script
    {
        std::size_t lexer::parse(string_view_type src)
        {
            std::size_t bracket_counter = 0;

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
                    auto parsed = parse_number(it, src.end());
                    push_lexeme<lexeme::constant>(std::move(parsed.first));
                    it = parsed.second;
                }
                else if(detail::is_identifier(ch, true))
                {
                    auto next = std::find_if_not(
                        std::next(it), src.end(),
                        [](char_type ch) { return detail::is_identifier(ch, false); }
                    );
                    string_view_type sv(it, next);
                    auto kw = get_keyword(sv);
                    if(kw.has_value())
                    {
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
                    while(!sv.empty())
                    {
                        auto op = get_operator(sv);
                        if(op.second == 0)
                            break;
                        if(op.first == operator_type::bracket_l)
                            ++bracket_counter;
                        if(op.first == operator_type::bracket_r)
                        {
                            // this right bracket indicates the end of script block
                            if(bracket_counter == 0)
                            {
                                // parsed characters don't include the ending bracket
                                return std::distance(src.begin(), it);
                            }

                            // normal right bracket for indexing
                            --bracket_counter;
                        }
                        sv = sv.substr(op.second);

                        push_lexeme<lexeme::operator_>(op.first);
                    }

                    if(!sv.empty())
                    {
                        throw lexer_error("unknown operator \"" + std::string(sv) + '\"');
                    }

                    it = next;
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

            return std::distance(src.begin(), it);
        }

        interpreter::string_type interpreter::run(string_view_type src, dynamic_format_arg_store args)
        {
            auto ex = compile(src);

            executor::context ctx(std::move(args));
            ex(ctx);

            return ctx.get_result();
        }
        executor interpreter::compile(string_view_type src)
        {
            auto lexemes = to_lexemes(src);
            return compile(lexemes);
        }
        executor interpreter::compile(std::span<const lexeme> lexemes)
        {
            return to_executor(lexemes);
        }

        std::vector<lexeme> interpreter::to_lexemes(string_view_type src)
        {
            lexer l;
            l.parse(src);
            return std::move(l).lexemes();
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
                using operator_position_data = std::pair<operator_type, iterator>;

                struct operator_finder
                {
                    operator_type target;

                    bool operator()(const lexeme& l) const
                    {
                        return
                            l.type() == lexeme_type::operator_ &&
                            l.as<lexeme::operator_>().get() == target;
                    }
                };

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
                    iterator colon = std::find_if(
                        begin, end, operator_finder{ .target = operator_type::colon }
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
                    if(count == 1)
                    {
                        auto& l = *begin;
                        if(l.type() == lexeme_type::constant)
                        {
                            auto& c = l.as<lexeme::constant>();
                            auto visitor = [](auto&& v)->bool
                            {
                                using T = std::remove_cvref_t<decltype(v)>;
                                if constexpr(std::is_same_v<T, string_type>)
                                {
                                    return !v.empty();
                                }
                                else
                                {
                                    return static_cast<bool>(v);
                                }
                            };

                            auto ex = std::make_unique<executor::constant<bool>>(
                                std::visit(visitor, c.to_underlying())
                            );
                            return std::make_pair(std::move(ex), end);
                        }
                        else if(l.type() == lexeme_type::argument)
                        {
                            auto& arg = l.as<lexeme::argument>();
                            std::unique_ptr<executor::argument> ex;
                            if(arg.is_indexed())
                                ex = std::make_unique<executor::argument>(arg.get_index());
                            else
                                ex = std::make_unique<executor::argument>(arg.get_string());
                            return std::make_pair(std::move(ex), end);
                        }
                    }
                    else if(
                        begin->type() == lexeme_type::operator_ &&
                        begin->as<lexeme::operator_>().get() == operator_type::not_
                    ) {
                        auto input = build_input(std::next(begin), end);
                        assert(input.second == end);

                        return std::make_pair(
                            std::make_unique<executor::logical_not>(std::move(input.first)),
                            end
                        );
                    }
                    else
                    {
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
                            raise_syntax_error("invalid Boolean expression");
                        }
                        auto lhs = build_input(begin, comp_it);
                        auto rhs = build_input(std::next(comp_it), end);

                        auto ex = get_comparator(
                            comp_it->as<lexeme::operator_>().get(),
                            std::move(lhs.first), std::move(rhs.first)
                        );

                        return std::make_pair(std::move(ex), rhs.second);
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
                            
                            if(!c.holds<string_type>())
                            {
                                raise_syntax_error("result type is not string");
                            }

                            return std::make_pair(
                                std::make_unique<executor::constant<string_type>>(c.get_string()),
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
                    assert(begin->type() == lexeme_type::argument);

                    auto& a = begin->as<lexeme::argument>();
                    std::vector<executor::argument::member_type> members;

                    ++begin;
                    auto it = begin;
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
                                members.push_back(attribute_name(std::move(id).get()));
                                it = std::next(identifier_it);
                            }
                            else
                                break;
                        }
                        else
                            break;
                    }

                    std::unique_ptr<executor::argument> ex;
                    if(a.is_indexed())
                        ex = std::make_unique<executor::argument>(a.get_index(), std::move(members));
                    else if(a.is_named())
                        ex = std::make_unique<executor::argument>(a.get_string(), std::move(members));
                    return std::make_pair(std::move(ex), it);
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

                                if(c.holds<string_type>())
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
    }
}
