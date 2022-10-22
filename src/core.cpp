#include <papilio/core.hpp>
#include <papilio/script.hpp>
#include <algorithm>
#include <cassert>
#include <cstring> // std::memset


namespace papilio
{
    namespace script
    {
        std::partial_ordering variable::compare(const variable& var) const
        {
            auto visitor = [](auto&& lhs, auto&& rhs)->std::partial_ordering
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(lhs)>;
                using U = std::remove_cvref_t<decltype(rhs)>;

                if constexpr(is_same_v<T, U>)
                {
                    return lhs <=> rhs;
                }
                else if constexpr(is_same_v<T, string_container> || is_same_v<U, string_container>)
                {
                    throw std::invalid_argument("invalid argument");
                }
                else
                {
                    using R = std::common_type_t<T, U>;
                    return static_cast<R>(lhs) <=> static_cast<R>(rhs);
                }
            };

            return std::visit(visitor, m_var, var.to_underlying());
        }

        bool variable::equal(
            const variable& var,
            float_type epsilon
        ) const noexcept {
            auto visitor = [epsilon](auto&& lhs, auto&& rhs)->bool
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(lhs)>;
                using U = std::remove_cvref_t<decltype(rhs)>;

                if constexpr(is_same_v<T, U>)
                {
                    if constexpr(std::floating_point<T>)
                    {
                        return std::abs(lhs - rhs) < epsilon;
                    }
                    else
                    {
                        return lhs == rhs;
                    }
                }
                else
                {
                    if constexpr(is_same_v<T, string_container> || is_same_v<U, string_container>)
                    {
                        return false;
                    }
                    else
                    {
                        using R = std::common_type_t<T, U>;
                        if constexpr(std::floating_point<R>)
                        {
                            return std::abs(static_cast<R>(lhs) - static_cast<R>(rhs)) < epsilon;
                        }
                        else
                        {
                            return static_cast<R>(lhs) == static_cast<R>(rhs);
                        }
                    }
                }
            };

            return std::visit(visitor, m_var, var.to_underlying());
        }
    }

    namespace detail
    {
        std_format_spec parse_std_format_spec(format_spec_parse_context& ctx)
        {
            std_format_spec result;

            auto it = ctx.rbegin();
            if(it == ctx.rend())
                return result;

            // type
            if('A' <= *it && *it <= 'z' && *it != 'L')
            {
                result.type_char = *it;
                ++it;
            }
            if(it == ctx.rend())
                return result;

            // locale
            if(*it == 'L')
            {
                result.use_locale = true;
                ++it;
            }
            if(it == ctx.rend())
                return result;

            if(is_digit(*it))
            {
                auto digit_end = std::find_if(
                    std::next(it), ctx.rend(), is_digit
                );
                std::string_view digits(
                    std::to_address(digit_end),
                    std::to_address(it)
                );
                if(digits.length() > 1 && digits[0] == '0')
                {
                    result.fill_zero = true;
                    digits = digits.substr(1);
                }
                auto conv_result = std::from_chars(
                    digits.data(), digits.data() + digits.size(),
                    result.width,
                    10
                );
                if(conv_result.ec != std::errc())
                {
                    throw invalid_format("invalid format width");
                }
                it = digit_end;
            }
            if(it == ctx.rend())
                return result;
            
            return result;
        }
    }

    namespace detail
    {
        static bool is_special_spec_ch(char32_t ch) noexcept
        {
            return
                ch == '}' ||
                ch == ']' ||
                ch == '{' ||
                ch == '[';
        }

        template <typename Iterator>
        std::optional<char32_t> next_char(Iterator current, Iterator end)
        {
            ++current;
            if(current == end)
                return std::nullopt;
            return *current;
        }

        template <typename Iterator>
        std::pair<std::size_t, Iterator> parse_spec_value(format_spec_parse_context& spec_ctx, Iterator begin, Iterator end)
        {
            if(*begin == '{')
            {
                auto field_end = script::find_field_end(std::next(begin), end);
                if(field_end == end)
                {
                    throw invalid_format("missing right brace: " + std::string(spec_ctx));
                }

                std::string_view view(
                    std::next(begin),
                    field_end
                );
                script::interpreter intp;
                std::optional<std::size_t> default_arg_id;
                if(!spec_ctx.manual_indexing())
                {
                    default_arg_id = spec_ctx.current_arg_id();
                    spec_ctx.next_arg_id();
                }
                auto [idx, arg_access] = intp.access(view, default_arg_id);

                assert(spec_ctx.has_store());
                format_arg arg = arg_access.access(spec_ctx.get_store()[idx]);

                return std::make_pair(
                    arg.as_variable().as<std::size_t>(),
                    ++field_end
                );
            }
            else
            {
                assert(is_digit(*begin));
                auto digit_end = std::find_if_not(
                    std::next(begin), end,
                    &is_digit
                );
                std::size_t val = 0;
                std::from_chars_result result = std::from_chars(
                    std::to_address(begin),
                    std::to_address(digit_end),
                    val,
                    10
                );
                assert(result.ptr == std::to_address(digit_end));
                if(result.ec != std::errc())
                {
                    throw std::system_error(std::make_error_code(result.ec));
                }

                return std::make_pair(
                    val, digit_end
                );
            }
        }
    }

    void common_format_spec::parse(format_spec_parse_context& spec_ctx)
    {
        std::string_view view = spec_ctx;
        reset();
        if(view.empty())
            return;

        struct parser_state
        {
            bool has_fill = false;
            bool width_parsed = false;
            bool dot_parsed = false;
        };
        parser_state state;

        for(auto it = view.begin(); it != view.end();)
        {
            utf8::codepoint cp = *it;

            if(cp == '0')
            {
                if(m_fill_zero)
                {
                    throw invalid_format("too many zeros");
                }
                m_fill_zero = true;
                ++it;
            }
            else if(detail::is_digit(cp) || cp == '{')
            {
                if(state.width_parsed)
                {
                    throw invalid_format("invalid format " + std::string(view));
                }
                std::size_t val = 0;
                std::tie(val, it) = detail::parse_spec_value(spec_ctx, it, view.end());

                m_width = val;
                state.width_parsed = true;
            }
            else if(cp == '.')
            {
                if(state.dot_parsed)
                {
                    throw invalid_format("too many precision values: " + std::string(view));
                }
                state.dot_parsed = true;

                std::optional next_ch = detail::next_char(it, view.end());
                if(!next_ch.has_value() || !(detail::is_digit(*next_ch) || *next_ch == '{'))
                {
                    throw invalid_format("invalid precision: " + std::string(view));
                }

                ++it;
                std::size_t val = 0;
                std::tie(val, it) = detail::parse_spec_value(spec_ctx, it, view.end());
                m_precision = val;
            }
            else
            {
                std::optional next_ch = detail::next_char(it, view.end());
                if(!next_ch.has_value())
                {
                    if(cp == 'L')
                        m_use_locale = true;
                    else if(is_align_spec(cp))
                        m_align = get_align(cp);
                    else if(cp == '#')
                        m_alternate_form = true;
                    else if(is_sign_spec(cp))
                        m_sign = get_sign(cp);
                    else
                        m_type_char = cp;
                    break;
                }
                else if(is_align_spec(*next_ch))
                {
                    m_fill = cp;
                    m_align = get_align(*next_ch);
                    ++it;
                }
                else if(*next_ch == '#')
                {
                    m_fill = cp;
                    m_alternate_form = true;
                    ++it;
                }
                else if(is_sign_spec(*next_ch))
                {
                    m_fill = cp;
                    m_sign = get_sign(*next_ch);
                    ++it;
                }

                ++it;
            }
        }
    }

    bool attribute_name::validate(string_view_type name) noexcept
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

    void format_arg::handle::copy(const handle& other) noexcept
    {
        other.ptr()->reset(ptr());
        m_has_value = true;
    }
    void format_arg::handle::destroy() noexcept
    {
        ptr()->~handle_impl_base();
        std::memset(m_storage, 0, sizeof(m_storage));
        m_has_value = false;
    }

    format_arg format_arg::index(const indexing_value& idx) const
    {
        auto visitor = [&idx]<typename T>(T&& v)->format_arg
        {
            using type = std::remove_cvref_t<T>;
            if constexpr(std::is_same_v<type, handle>)
            {
                return v.index(idx);
            }
            else
            {
                return accessor_traits<type>::get_arg(
                    std::forward<T>(v),
                    idx
                );
            }
        };
        return std::visit(visitor, m_val);
    }
    format_arg format_arg::attribute(const attribute_name& attr) const
    {
        auto visitor = [&attr]<typename T>(T&& v)->format_arg
        {
            using type = std::remove_cvref_t<T>;

            if constexpr(std::is_same_v<type, handle>)
            {
                return v.attribute(attr);
            }
            else
            {
                return accessor_traits<type>::get_attr(v, attr);
            }
        };
        return std::visit(visitor, m_val);
    }

    script::variable format_arg::as_variable() const
    {
        using script::variable;

        auto visitor = [](auto&& v)->script::variable
        {
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(std::is_same_v<T, utf8::codepoint>)
            {
                return variable(string_container(v, 1));
            }
            if constexpr(std::integral<T>)
            {
                return static_cast<variable::int_type>(v);
            }
            else if constexpr(std::floating_point<T>)
            {
                return static_cast<variable::float_type>(v);
            }
            else if constexpr(std::is_same_v<T, string_container>)
            {
                return variable(v);
            }
            else
            {
                throw std::invalid_argument("invalid type");
            }
        };

        return std::visit(visitor, m_val);
    }

    format_arg format_arg_access::access(format_arg arg) const
    {
        format_arg result = arg;

        auto visitor = [&result](auto&& v)->format_arg
        {
            using std::is_same_v;
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(is_same_v<T, indexing_value>)
            {
                return result.index(v);
            }
            else if constexpr(is_same_v<T, attribute_name>)
            {
                return result.attribute(v);
            }
        };

        for(const auto& i : m_members)
        {
            result = std::visit(visitor, i);
        }

        return result;
    }

    const format_arg& dynamic_format_arg_store::get(size_type i) const
    {
        if(i >= m_args.size())
            throw std::out_of_range("index out of range");
        return m_args[i];
    }
    const format_arg& dynamic_format_arg_store::get(string_view_type key) const
    {
        auto it = m_named_args.find(key);
        if(it == m_named_args.end())
            throw std::out_of_range("invalid named argument");
        return it->second;
    }
    const format_arg& dynamic_format_arg_store::get(const indexing_value& idx) const
    {
        auto visitor = [&](auto&& v)->const format_arg&
        {
            using std::is_same_v;
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(is_same_v<T, indexing_value::index_type>)
            {
                size_type i = static_cast<size_type>(v);
                if(i >= m_args.size())
                    throw std::out_of_range("argument index out of range");
                return m_args[i];
            }
            else if constexpr(is_same_v<T, string_container>)
            {
                auto it = m_named_args.find(v);
                if(it == m_named_args.end())
                    throw std::out_of_range("invalid named argument");
                return it->second;
            }
            else
            {
                throw std::invalid_argument("invalid indexing value");
            }
        };

        return std::visit(visitor, idx.to_underlying());
    }

    bool dynamic_format_arg_store::check(const indexing_value& idx) const noexcept
    {
        auto visitor = [this](auto&& v)->bool
        {
            using std::is_same_v;
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(is_same_v<T, indexing_value::index_type>)
                return  v < m_args.size();
            else if constexpr(is_same_v<T, string_container>)
                return m_named_args.contains(string_view_type(v));
            else
                return false;
        };

        return std::visit(visitor, idx.to_underlying());
    }

    format_parse_context::format_parse_context(string_view_type str, const dynamic_format_arg_store& store)
        : m_view(str), m_store(&store) {
        m_it = str.begin();
    }
}
