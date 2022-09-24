#include <papilio/script.hpp>
#include <papilio/core.hpp>


namespace papilio
{
    namespace script
    {
        void lexer::parse(string_view_type src)
        {
            for(auto it = src.begin(); it != src.end();)
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
                    string_view_type sv(it, next);
                    auto op = get_operator(sv);
                    if(op.has_value())
                    {
                        push_lexeme<lexeme::operator_>(*op);
                    }
                    else
                    {
                        throw lexer_error("lexer error");
                    }

                    it = next;
                }
                else
                {
                    throw lexer_error("lexer error");
                }
            }
        }
    }
}
