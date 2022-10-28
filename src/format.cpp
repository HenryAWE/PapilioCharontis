#include <papilio/format.hpp>


namespace papilio
{
    void format_parser::parse(string_view_type str, const dynamic_format_arg_store& store)
    {
        string_type seg_str;
        script::lexer lex;
        script::interpreter intp;

        format_parse_context ctx(str, store);

        iterator it = ctx.begin();
        for(; it != ctx.end(); it = ctx.begin())
        {
            char_type ch = *it;
            if(ch == '{')
            {
                iterator next_it = std::next(it);
                if(next_it == ctx.end())
                {
                    throw std::runtime_error("missing replacement field");
                }

                if(*next_it == '{')
                {
                    seg_str += '{';
                    ctx.advance_to(std::next(next_it));
                    continue;
                }
                else
                {
                    if(!seg_str.empty())
                    {
                        push_segment<plain_text>(std::move(seg_str));
                        seg_str = string_type(); // avoid warning
                    }

                    std::optional<std::size_t> default_arg_id = std::nullopt;
                    if(!ctx.manual_indexing())
                        default_arg_id = ctx.current_arg_id();
                    iterator field_begin = std::next(it);
                    lex.clear();
                    auto result = lex.parse(
                        string_view_type(field_begin, ctx.end()),
                        script::lexer_mode::replacement_field,
                        default_arg_id
                    );
                    if(result.default_arg_idx_used)
                        ctx.next_arg_id();
                    else
                        ctx.enable_manual_indexing();
                    iterator arg_end = std::next(field_begin, result.parsed_char);
                    if(arg_end == ctx.end())
                    {
                        throw std::runtime_error("missing right brace ('}')");
                    }

                    auto pred = [counter = std::size_t(0)](char_type ch) mutable
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
                    iterator fmt_begin = arg_end;
                    if(*arg_end == ':')
                        ++fmt_begin;
                    iterator field_end = std::find_if(fmt_begin, ctx.end(), pred);
                    if(field_end == ctx.end())
                    {
                        throw std::runtime_error("missing right brace ('}')");
                    }

                    auto arg_access = build_arg_access(lex.lexemes());
                    push_segment<replacement_field>(
                        std::move(arg_access.first),
                        std::move(arg_access.second),
                        string_type(fmt_begin, field_end)
                    );

                    ctx.advance_to(std::next(field_end));
                }
            }
            else if(ch == '}')
            {
                iterator next_it = std::next(it);
                if(next_it != ctx.end() && *next_it == '}')
                {
                    seg_str += '}';
                    ctx.advance_to(std::next(next_it));
                }
                else
                {
                    throw std::runtime_error("invalid right brace ('}')");
                }
            }
            else if(ch == '[')
            {
                iterator next_it = std::next(it);
                if(next_it == ctx.end())
                {
                    throw std::runtime_error("missing script block");
                }

                if(*next_it == '[')
                {
                    seg_str += '[';
                    ctx.advance_to(std::next(next_it));
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
                    lex.clear();
                    auto result = lex.parse(
                        src,
                        script::lexer_mode::script_block
                    );
                    iterator script_end = std::next(next_it, result.parsed_char);
                    if(script_end == ctx.end() || *script_end != ']')
                    {
                        throw std::runtime_error("missing right bracket (']')");
                    }

                    push_segment<script_block>(intp.compile(lex.lexemes()));
                    lex.clear();

                    ctx.advance_to(std::next(script_end));
                }
            }
            else if(ch == ']')
            {
                iterator next_it = std::next(it);
                if(next_it != ctx.end() && *next_it == ']')
                {
                    seg_str += ']';
                    ctx.advance_to(std::next(next_it));
                }
                else
                {
                    throw std::runtime_error("invalid right brace ('}')");
                }
            }
            else
            {
                seg_str += ch;
                ctx.advance_to(std::next(it));
            }
        }

        if(!seg_str.empty())
            push_segment<plain_text>(std::move(seg_str));
    }

    std::pair<indexing_value, format_arg_access> format_parser::build_arg_access(
        std::span<const script::lexeme> lexemes
    ) {
        script::interpreter intp;
        return intp.access(lexemes);
    }

    std::string vformat(std::string_view fmt, const dynamic_format_arg_store& store)
    {
        format_parser parser;
        parser.parse(fmt, store);

        std::string result;
        basic_format_context fmt_ctx(std::back_inserter(result), store);

        auto segments = parser.segments();
        auto visitor = [&fmt_ctx](const auto& seg)
        {
            seg.format(fmt_ctx);
        };
        for(const auto& seg : segments)
        {
            std::visit(visitor, seg);
        }

        return result;
    }

    namespace detail
    {
        std::pair<std::size_t, std::size_t> calc_fill_width(format_align align, std::size_t width, std::size_t current)
        {
            if(width <= current)
                return std::make_pair(0, 0);

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            std::size_t to_fill = width - current;

            using enum format_align;
            switch(align)
            {
            case left:
                fill_back = to_fill;
                break;

            [[likely]] case right:
                fill_front = to_fill;
                break;

            case middle:
                fill_front = to_fill / 2; // floor(to_fill / 2)
                fill_back = to_fill / 2 + to_fill % 2; // ceil(to_fill / 2)
                break;
            }

            return std::make_pair(fill_front, fill_back);
        }
    }
}
