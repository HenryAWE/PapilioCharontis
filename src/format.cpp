#include <papilio/format.hpp>


namespace papilio
{
    void format_parser::parse(string_view_type str)
    {
        string_type seg_str;
        script::lexer lex;
        script::interpreter intp;
        for(auto it = str.begin(); it != str.end();)
        {
            char_type ch = *it;
            if(ch == '{')
            {
                iterator next_it = std::next(it);
                if(next_it == str.end())
                {
                    throw std::runtime_error("missing replacement field");
                }

                if(*next_it == '{')
                {
                    seg_str += '{';
                    it = std::next(next_it);
                    continue;
                }
                else
                {
                    if(!seg_str.empty())
                    {
                        push_segment<plain_text>(std::move(seg_str));
                        seg_str = string_type(); // avoid warning
                    }

                    auto pred = [counter = std::size_t(0)](char_type ch) mutable
                    {
                        if(ch == '{')
                            ++counter;
                        else if(ch == '}')
                        {
                            if(counter == 0)
                                return true;
                            --counter;
                        }

                        return false;
                    };
                    iterator field_begin = next_it;
                    iterator field_end = std::find_if(
                        field_begin, str.end(), pred
                    );
                    if(field_end == str.end())
                    {
                        throw std::runtime_error("missing right brace ('}')");
                    }

                    string_type field_str(field_begin, field_end);
                    push_segment<replacement_field>(std::move(field_str));

                    it = std::next(field_end);
                }
            }
            else if(ch == '}')
            {
                iterator next_it = std::next(it);
                if(next_it != str.end() && *next_it == '}')
                {
                    seg_str += '}';
                    it = std::next(next_it);
                }
                else
                {
                    throw std::runtime_error("invalid right brace ('}')");
                }
            }
            else if(ch == '[')
            {
                iterator next_it = std::next(it);
                if(next_it == str.end())
                {
                    throw std::runtime_error("missing script block");
                }

                if(*next_it == '[')
                {
                    seg_str += '[';
                    it = std::next(next_it);
                    continue;
                }
                else
                {
                    if(!seg_str.empty())
                    {
                        push_segment<plain_text>(std::move(seg_str));
                        seg_str = string_type(); // avoid warning
                    }

                    string_view_type src(next_it, str.end());
                    std::size_t parsed = lex.parse(src);
                    iterator script_end = std::next(next_it, parsed);
                    if(script_end == str.end() || *script_end != ']')
                    {
                        throw std::runtime_error("missing right bracket (']')");
                    }

                    push_segment<script_block>(intp.compile(lex.lexemes()));
                    lex.clear();

                    it = std::next(script_end);
                }
            }
            else if(ch == ']')
            {
                iterator next_it = std::next(it);
                if(next_it != str.end() && *next_it == ']')
                {
                    seg_str += ']';
                    it = std::next(next_it);
                }
                else
                {
                    throw std::runtime_error("invalid right brace ('}')");
                }
            }
            else
            {
                seg_str += ch;
                ++it;
            }
        }

        if(!seg_str.empty())
            push_segment<plain_text>(std::move(seg_str));
    }
}
