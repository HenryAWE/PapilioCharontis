#include <papilio/core.hpp>


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
                else if constexpr(is_same_v<T, string_type> || is_same_v<U, string_type>)
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
                    if constexpr(is_same_v<T, string_type> || is_same_v<U, string_type>)
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

    format_arg format_arg::index(const indexing_value& idx) const
    {
        auto visitor = [&](auto&& v)->format_arg
        {
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(string_like<T>)
            {
                if(idx.is_index())
                {
                    auto i = idx.as_index();

                    if constexpr(std::is_same_v<T, string_type>)
                    {
                        if(i < 0)
                            return format_arg(utf8::rindex(v, -(i + 1)));
                        else
                            return format_arg(utf8::index(v, i));
                    }
                    else
                    {
                        string_view_type sv(v);
                        if(i < 0)
                            return format_arg(utf8::rindex(sv, -(i + 1)));
                        else
                            return format_arg(utf8::index(sv, i));
                    }
                }
                else if(idx.is_slice())
                {
                    const auto& s = idx.as_slice();

                    if constexpr(std::is_same_v<T, string_type>)
                    {
                        return format_arg(utf8::substr(v, s));
                    }
                    else
                    {
                        string_view_type sv(v);
                        return format_arg(utf8::substr(sv, s));
                    }
                }
            }

            invalid_index();
        };
        return std::visit(visitor, m_val);
    }
    format_arg format_arg::attribute(attribute_name name) const
    {
        auto visitor = [&](auto&& v)->format_arg
        {
            using namespace std::literals;
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(string_like<T>)
            {
                string_view_type str(v);
                if(name == "length"sv)
                {
                    return format_arg(utf8::strlen(str));
                }
                else if(name == "size"sv)
                {
                    return format_arg(str.size());
                }
            }

            invalid_attribute();
        };
        return std::visit(visitor, m_val);
    }

    script::variable format_arg::as_variable() const
    {
        using script::variable;

        auto visitor = [](auto&& v)->script::variable
        {
            using T = std::remove_cvref_t<decltype(v)>;

            if constexpr(std::is_same_v<T, char_type>)
            {
                return string_type(1, v);
            }
            if constexpr(std::integral<T>)
            {
                return static_cast<variable::int_type>(v);
            }
            else if constexpr(std::floating_point<T>)
            {
                return static_cast<variable::float_type>(v);
            }
            else if constexpr(string_like<T>)
            {
                return string_view_type(v);
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
                return m_args[static_cast<size_type>(v)];
            }
            else if constexpr(is_same_v<T, string_type>)
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
                return check(v);
            else if constexpr(is_same_v<T, string_type>)
                return check(string_view_type(v));
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
