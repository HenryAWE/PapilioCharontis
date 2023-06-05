#include <papilio/core.hpp>
#include <papilio/script.hpp>
#include <algorithm>
#include <ranges>
#include <cassert>
#include <cstring> // std::memset


namespace papilio
{
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

        static bool is_spec_type_char(char32_t ch) noexcept
        {
            char32_t type_char_list[] = {
                's', // string
                'b', 'B', 'c', 'o', 'd', 'x', 'X', // integer and char
                'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G', // floating point
                'p' // pointer
            };

            namespace stdr = std::ranges;
            return stdr::find(type_char_list, ch) != stdr::end(type_char_list);
        }

        // WARNING: Please remember that the base iterator of a reverse one
        // points to the position after the reverse iterator
        std::string_view to_string_view(
            string_container::const_reverse_iterator start,
            string_container::const_reverse_iterator stop
        ) {
            auto begin = stop.base();
            auto end = start.base();

            using std::to_address;
            return std::string_view(
                to_address(begin),
                to_address(end)
            );
        }

        template <typename ReverseIterator>
        std::pair<ReverseIterator, std::optional<std::string_view>> rparse_value(
            ReverseIterator start,
            ReverseIterator stop,
            std::size_t& val_out
        ) {
            char32_t last_ch = *start;
            if(last_ch == '}')
            {
                ++start;
                auto field_end = start;
                auto field_begin = script::rfind_field_begin(start, stop);
                ++field_end;
                std::string_view view = to_string_view(
                    field_begin,
                    start
                );

                return std::make_pair(field_end, view);
            }
            else if(is_digit(last_ch))
            {
                auto next_it = start;
                for(auto it = start; it != stop; ++it)
                {
                    char32_t ch = *it;
                    if(U'1' <= ch && ch <= U'9')
                        next_it = it;
                    else if(ch != U'0')
                        break;
                }

                ++next_it;
                std::string_view view = to_string_view(
                    start,
                    next_it
                );

                std::size_t value = 0;
                auto conv_result = std::from_chars(
                    view.data(),
                    view.data() + view.size(),
                    value,
                    10
                );

                val_out = value;
                return std::make_pair(next_it, std::nullopt);
            }

            return std::make_pair(start, std::nullopt);
        }

        class common_format_spec_parse_context
        {
        public:
            using char_type = char;
            using string_view_type = std::basic_string_view<char>;
            using iterator = string_view_type::const_iterator;
            using reverse_iterator = string_view_type::const_reverse_iterator;

            void parse(format_spec_parse_context& ctx)
            {
                parse_string(string_view_type(ctx));
                eval_dyn_val(ctx);
            }
            void parse_string(string_container view)
            {
                if(view.empty())
                    return;
                assert(view.is_borrowed());

                auto it = view.crbegin();
                auto rend = view.crend();

#define PAPILIO_CHECK_SPEC_ITER(iter) if(iter == rend) return;

                if(utf8::codepoint cp = *it; is_spec_type_char(cp))
                {
                    spec.type_char(cp);
                    ++it;
                }

                PAPILIO_CHECK_SPEC_ITER(it);
                if(*it == 'L')
                {
                    spec.use_locale(true);
                    ++it;
                }

                PAPILIO_CHECK_SPEC_ITER(it);
                {
                    std::optional<string_view_type> dyn_val;
                    std::size_t static_val = 0;
                    auto tmp_it = it;
                    std::tie(tmp_it, dyn_val) = rparse_value(it, rend, static_val);
                    if(it == tmp_it)
                        goto parse_value_end;
                    it = tmp_it;

                    bool has_dot = false;
                    if(it != rend && *it == U'.')
                    {
                        has_dot = true;
                        ++it;
                    }

                    if(!has_dot)
                    {
                        if(dyn_val.has_value())
                            m_dyn_width = dyn_val;
                        else
                            spec.width(static_val);
                    }
                    else
                    {
                        if(dyn_val.has_value())
                            m_dyn_precision = dyn_val;
                        else
                            spec.precision(static_val);

                        PAPILIO_CHECK_SPEC_ITER(it);
                        std::tie(it, dyn_val) = rparse_value(it, rend, static_val);

                        if(dyn_val.has_value())
                            m_dyn_width = dyn_val;
                        else
                            spec.width(static_val);
                    }
                }
            parse_value_end:

                PAPILIO_CHECK_SPEC_ITER(it);
                if(*it == U'0')
                {
                    spec.fill_zero(true);
                    ++it;
                }
                PAPILIO_CHECK_SPEC_ITER(it);
                if(*it == U'#')
                {
                    spec.alternate_form(true);
                    ++it;
                }
                PAPILIO_CHECK_SPEC_ITER(it);
                if(char32_t ch = *it; common_format_spec::is_sign_spec(ch))
                {
                    spec.sign(common_format_spec::get_sign(ch));
                    ++it;
                }
                PAPILIO_CHECK_SPEC_ITER(it);
                if(char32_t ch = *it; common_format_spec::is_align_spec(ch))
                {
                    spec.align(common_format_spec::get_align(ch));
                    ++it;
                }
                PAPILIO_CHECK_SPEC_ITER(it);
                spec.fill(*it);
                ++it;

                if(it != rend)
                {
                    throw invalid_format(
                        "invalid format specification for common type: " +
                        std::string(view)
                    );
                }

#undef PAPILIO_CHECK_SPEC_ITER
            }

            void eval_dyn_val(format_spec_parse_context& spec_ctx)
            {
                script::interpreter intp;
                if(m_dyn_width.has_value())
                {
                    std::optional<std::size_t> default_arg_id;
                    if(!spec_ctx.manual_indexing())
                    {
                        default_arg_id = spec_ctx.current_arg_id();
                        spec_ctx.next_arg_id();
                    }
                    auto [idx, acc] = intp.access(*m_dyn_width, default_arg_id);
                    std::size_t val = acc.access(spec_ctx.get_store().get(idx)).as_variable().as<std::size_t>();
                    spec.width(val);
                }
                if(m_dyn_precision.has_value())
                {
                    std::optional<std::size_t> default_arg_id;
                    if(!spec_ctx.manual_indexing())
                    {
                        default_arg_id = spec_ctx.current_arg_id();
                        spec_ctx.next_arg_id();
                    }
                    auto [idx, acc] = intp.access(*m_dyn_precision, default_arg_id);
                    std::size_t val = acc.access(spec_ctx.get_store().get(idx)).as_variable().as<std::size_t>();
                    spec.precision(val);
                }
            }

            common_format_spec spec;

        private:
            std::optional<string_view_type> m_dyn_width;
            std::optional<string_view_type> m_dyn_precision;
            bool m_has_fill_zero = false;
        };

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
        detail::common_format_spec_parse_context parse_ctx;
        parse_ctx.parse(spec_ctx);

        *this = parse_ctx.spec;
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

    namespace detail
    {
        const format_arg& format_arg_store_base::get(const indexing_value& idx) const
        {
            auto visitor = [&](auto&& v)->const format_arg&
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(is_same_v<T, indexing_value::index_type>)
                {
                    if(v < 0)
                        raise_index_out_of_range();
                    size_type i = static_cast<size_type>(v);
                    return get(i);
                }
                else if constexpr(is_same_v<T, string_container>)
                {
                    return get(string_view_type(v));
                }
                else
                {
                    throw std::invalid_argument("invalid indexing value");
                }
            };

            return std::visit(visitor, idx.to_underlying());
        }

        bool format_arg_store_base::check(size_type i) const noexcept
        {
            return i < size();
        }
        bool format_arg_store_base::check(const indexing_value& idx) const noexcept
        {
            auto visitor = [this](auto&& v)->bool
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(is_same_v<T, indexing_value::index_type>)
                {
                    if(v < 0)
                        return false;
                    return check(static_cast<size_type>(v));
                }
                else if constexpr(is_same_v<T, string_container>)
                {
                    return check(string_view_type(v));
                }
                else
                {
                    return false;
                }
            };

            return std::visit(visitor, idx.to_underlying());
        }

        void format_arg_store_base::raise_index_out_of_range()
        {
            throw std::out_of_range("index out of range");
        }
        void format_arg_store_base::raise_invalid_named_argument()
        {
            throw std::out_of_range("invalid named argument");
        }
    }

    const format_arg& mutable_format_arg_store::get(size_type i) const
    {
        if(i >= m_args.size())
            raise_index_out_of_range();
        return m_args[i];
    }
    const format_arg& mutable_format_arg_store::get(string_view_type key) const
    {
        auto it = m_named_args.find(key);
        if(it == m_named_args.end())
            raise_invalid_named_argument();
        return it->second;
    }

    bool mutable_format_arg_store::check(string_view_type k) const noexcept
    {
        return m_named_args.contains(k);
    }

    format_parse_context::format_parse_context(string_view_type str, const dynamic_format_arg_store& store)
        : m_view(str), m_store(store) {
        m_it = str.begin();
    }
}
