#pragma once

#include "core.hpp"
#include <concepts>
#include <system_error>
#include <charconv>
#include <sstream>
#include "detail/iterstream.hpp"


namespace papilio
{
    namespace detail
    {
        template <typename T>
        format_arg handle_impl<T>::index(const indexing_value& idx) const
        {
            return accessor_traits<T>::get_arg(*m_ptr, idx);
        }
        template <typename T>
        format_arg handle_impl<T>::attribute(const attribute_name& attr) const
        {
            return accessor_traits<T>::get_attr(*m_ptr, attr);
        }

        template <typename T>
        void handle_impl<T>::format(format_spec_parse_context& spec, dynamic_format_context& ctx) const
        {
            using traits = formatter_traits<T>;
            traits::format(spec, *m_ptr, ctx);
        }
    }

    template <typename T>
    template <typename Context>
    constexpr bool formatter_traits<T>::has_formatter() noexcept
    {
        return requires(formatter<T> f, format_spec_parse_context& spec, const type& val, Context& ctx)
        {
            typename formatter<T>;
            f.parse(spec);
            f.format(val, ctx);
        };
    }

    template <typename T>
    template <typename Context>
    void formatter_traits<T>::format(format_spec_parse_context& spec, const type& val, Context& ctx)
    {
        if constexpr(has_formatter<Context>())
        {
            formatter_type fmt;
            fmt.parse(spec);
            fmt.format(val, ctx);
        }
        else if constexpr(detail::has_ostream_support_helper<T>)
        {
            using context_traits = format_context_traits<Context>;
            using streambuf_type = detail::basic_iterbuf<
                char,
                typename context_traits::iterator
            >;

            streambuf_type sbuf(context_traits::out(ctx));

            std::ostream os(&sbuf);
            std::string_view view(spec);
            if(view == "L")
                os.imbue(ctx.getloc());
            else if(!view.empty())
            {
                throw invalid_format("invalid format");
            }

            os << val;

            context_traits::advance_to(ctx, sbuf.get());
        }
        else
        {
            throw std::runtime_error(
                std::string("no formatter for ") + typeid(type).name()
            );
        }
    }

    template <>
    struct accessor<string_container>
    {
        using has_index = void;
        using has_slice = void;

        static utf8::codepoint get(const string_container& str, indexing_value::index_type i)
        {
            return str[i];
        }

        static string_container get(const string_container& str, slice s)
        {
            return str.substr(s);
        }

        static format_arg get_attr(const string_container& str, const attribute_name& attr)
        {
            using namespace std::literals;

            if(attr == "length"sv)
                return str.length();
            else if(attr == "size")
                return str.size();
            else
                throw invalid_attribute(attr);
        }
    };

    template <typename T>
    template <typename U>
    format_arg accessor_traits<T>::get_arg(U&& object, const indexing_value& idx)
    {
        auto visitor = [&object]<typename Arg>(Arg&& v)->format_arg
        {
            using result_type = decltype(
                index_handler(std::forward<U>(object), std::forward<Arg>(v))
            );
            if constexpr(std::is_void_v<result_type>)
            {
                // trigger exception
                get(std::forward<U>(object), std::forward<Arg>(v));
                // placeholder
                return format_arg();
            }
            else
            {
                return format_arg(get(std::forward<U>(object), std::forward<Arg>(v)));
            }
        };
        return std::visit(visitor, idx.to_underlying());
    }
    template <typename T>
    template <typename U>
    format_arg  accessor_traits<T>::get_attr(U&& object, const attribute_name& attr)
    {
        if constexpr(requires() { accessor_type::get_attr(std::forward<U>(object), attr); })
        {
            return accessor_type::get_attr(std::forward<U>(object), attr);
        }
        else
            throw invalid_attribute(attr);
    }

    template <typename Context>
    void format_arg::format(format_spec_parse_context& spec, Context& ctx)
    {
        auto visitor = [&spec, &ctx]<typename T> (const T& v)
        {
            if constexpr(std::is_same_v<T, handle>)
            {
                if constexpr(std::is_same_v<Context, dynamic_format_context>)
                {
                    v.format(spec, ctx);
                }
                else
                {
                    dynamic_format_context dyn_ctx(ctx);
                    v.format(spec, dyn_ctx);
                }
            }
            else
            {
                using traits = formatter_traits<T>;
                traits::format(spec, v, ctx);
            }
        };

        std::visit(visitor, m_val);
    }
}
