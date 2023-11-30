#include <papilio/core.hpp>
#include <papilio/script.hpp>
#include <algorithm>


namespace papilio
{
    format_arg format_arg::index(const indexing_value& idx) const
    {
        auto visitor = [&idx]<typename T>(T&& v) -> format_arg
        {
            using type = std::remove_cvref_t<T>;
            if constexpr(std::is_same_v<type, handle>)
            {
                return v.index(idx);
            }
            else
            {
                return idx.visit(
                    [&](auto&& i) { return accessor_traits<type>::template index<format_arg>(v, i); }
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
                return accessor_traits<type>::template attribute<format_arg>(v, attr);
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

            if constexpr(std::is_same_v<T, utf::codepoint>)
            {
                return variable(utf::string_container(1, v));
            }
            if constexpr(std::integral<T>)
            {
                return static_cast<variable::int_type>(v);
            }
            else if constexpr(std::floating_point<T>)
            {
                return static_cast<variable::float_type>(v);
            }
            else if constexpr(std::is_same_v<T, utf::string_container>)
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

    format_arg chained_access::access(format_arg arg) const
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
        const format_arg& format_args_base::get(const indexing_value& idx) const
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
                else if constexpr(is_same_v<T, utf::string_container>)
                {
                    return get(string_view_type(v));
                }
                else
                {
                    throw std::invalid_argument("invalid indexing value");
                }
            };

            return idx.visit(visitor);
        }

        bool format_args_base::check(size_type i) const noexcept
        {
            return i < indexed_size();
        }
        bool format_args_base::check(const indexing_value& idx) const noexcept
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
                else if constexpr(is_same_v<T, utf::string_container>)
                {
                    return check(string_view_type(v));
                }
                else
                {
                    return false;
                }
            };

            return idx.visit(visitor);
        }

        void format_args_base::raise_index_out_of_range()
        {
            throw std::out_of_range("index out of range");
        }
        void format_args_base::raise_invalid_named_argument()
        {
            throw std::out_of_range("invalid named argument");
        }
    }

    const format_arg& mutable_format_args::get(size_type i) const
    {
        if(i >= m_args.size())
            raise_index_out_of_range();
        return m_args[i];
    }
    const format_arg& mutable_format_args::get(string_view_type key) const
    {
        auto it = m_named_args.find(key);
        if(it == m_named_args.end())
            raise_invalid_named_argument();
        return it->second;
    }

    bool mutable_format_args::check(string_view_type k) const noexcept
    {
        return m_named_args.contains(k);
    }
}
