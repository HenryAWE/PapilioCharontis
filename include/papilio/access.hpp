#pragma once

#include <variant>
#include "fmtfwd.hpp"
#include "utility.hpp"
#include "utf/codepoint.hpp"
#include "utf/string.hpp"
#include "container.hpp"

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat"
#    pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

namespace papilio
{
PAPILIO_EXPORT template <typename CharT>
class basic_indexing_value
{
public:
    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;
    using index_type = ssize_t;
    using variant_type = std::variant<
        index_type,
        slice,
        string_container_type>;

    basic_indexing_value() = delete;
    basic_indexing_value(const basic_indexing_value&) = default;
    basic_indexing_value(basic_indexing_value&&) = default;

    basic_indexing_value(index_type index)
        : m_val(index) {}

    basic_indexing_value(slice s)
        : m_val(s) {}

    basic_indexing_value(string_container_type key)
        : m_val(std::move(key)) {}

    template <basic_string_like<CharT> String>
    basic_indexing_value(String&& key)
        : m_val(std::in_place_type<string_container_type>, std::forward<String>(key))
    {}

    template <basic_string_like<CharT> String>
    basic_indexing_value(independent_t, String&& key)
        : m_val(std::in_place_type<string_container_type>, independent, std::forward<String>(key))
    {}

    template <typename T>
    [[nodiscard]]
    bool holds() const noexcept
    {
        return std::holds_alternative<T>(m_val);
    }

    [[nodiscard]]
    bool holds_index() const noexcept
    {
        return holds<index_type>();
    }

    [[nodiscard]]
    bool holds_slice() const noexcept
    {
        return holds<slice>();
    }

    [[nodiscard]]
    bool holds_string() const noexcept
    {
        return holds<string_container_type>();
    }

    [[nodiscard]]
    index_type as_index() const noexcept
    {
        PAPILIO_ASSERT(holds<index_type>());
        return *std::get_if<index_type>(&m_val);
    }

    [[nodiscard]]
    slice as_slice() const noexcept
    {
        PAPILIO_ASSERT(holds<slice>());
        return *std::get_if<slice>(&m_val);
    }

    [[nodiscard]]
    const string_container_type& as_string() const noexcept
    {
        PAPILIO_ASSERT(holds<string_container_type>());
        return *std::get_if<string_container_type>(&m_val);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& vis) const
    {
        return std::visit(std::forward<Visitor>(vis), m_val);
    }

private:
    variant_type m_val;
};

PAPILIO_EXPORT using indexing_value = basic_indexing_value<char>;
PAPILIO_EXPORT using windexing_value = basic_indexing_value<wchar_t>;

PAPILIO_EXPORT template <typename CharT>
class basic_attribute_name
{
public:
    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;

    basic_attribute_name() = delete;
    basic_attribute_name(const basic_attribute_name&) = default;
    basic_attribute_name(basic_attribute_name&&) noexcept = default;

    template <basic_string_like<CharT> String>
    basic_attribute_name(String&& str) noexcept(std::is_nothrow_constructible_v<string_container_type, String>)
        : m_name(std::forward<String>(str))
    {}

    template <basic_string_like<CharT> String>
    basic_attribute_name(independent_t, String&& str) noexcept(std::is_nothrow_constructible_v<string_container_type, String>)
        : m_name(independent, std::forward<String>(str))
    {}

    template <typename... Args>
    basic_attribute_name(std::in_place_t, Args&&... args)
        : m_name(std::forward<Args>(args)...)
    {}

    bool operator==(const basic_attribute_name& rhs) const noexcept = default;

    friend bool operator==(const basic_attribute_name& lhs, const string_type& rhs) noexcept
    {
        return lhs.m_name == rhs;
    }

    friend bool operator==(const string_type& lhs, const basic_attribute_name& rhs) noexcept
    {
        return lhs == rhs.m_name;
    }

    friend bool operator==(const basic_attribute_name& lhs, string_view_type rhs) noexcept
    {
        return lhs.m_name == rhs;
    }

    friend bool operator==(string_view_type lhs, const basic_attribute_name& rhs) noexcept
    {
        return lhs == rhs.m_name;
    }

    friend bool operator==(const basic_attribute_name& lhs, const char* rhs) noexcept
    {
        return lhs.m_name == rhs;
    }

    friend bool operator==(const char* lhs, const basic_attribute_name& rhs) noexcept
    {
        return lhs == rhs.m_name;
    }

    [[nodiscard]]
    const string_container_type& name() const noexcept
    {
        return m_name;
    }

    operator string_view_type() const noexcept
    {
        return static_cast<string_view_type>(m_name);
    }

private:
    string_container_type m_name;
};

PAPILIO_EXPORT using attribute_name = basic_attribute_name<char>;
PAPILIO_EXPORT using wattribute_name = basic_attribute_name<wchar_t>;

PAPILIO_EXPORT class invalid_attribute_base : public std::invalid_argument
{
public:
    invalid_attribute_base(const invalid_attribute_base&) = default;

protected:
    invalid_attribute_base()
        : invalid_argument("invalid attribute") {}
};

PAPILIO_EXPORT template <typename CharT>
class basic_invalid_attribute : public invalid_attribute_base
{
public:
    using char_type = CharT;
    using attribute_name_type = basic_attribute_name<CharT>;

    basic_invalid_attribute(const attribute_name_type& attr)
        : m_attr(independent, attr.name()) {}

    [[nodiscard]]
    const basic_attribute_name<CharT>& name() const noexcept
    {
        return m_attr;
    }

private:
    attribute_name_type m_attr;
};

using invalid_attribute = basic_invalid_attribute<char>;

PAPILIO_EXPORT template <typename CharT>
[[noreturn]]
void throw_invalid_attribute(const basic_attribute_name<CharT>& attr)
{
    throw basic_invalid_attribute<CharT>(attr);
}

PAPILIO_EXPORT template <typename T, typename Context = format_context>
struct accessor
{};

namespace detail
{
    class accessor_traits_base
    {
    protected:
        [[noreturn]]
        static void integer_index_unavailable()
        {
            throw std::runtime_error("index unavailable");
        }

        [[noreturn]]
        static void string_index_unavailable()
        {
            throw std::runtime_error("index unavailable");
        }

        [[noreturn]]
        static void slice_index_unavailable()
        {
            throw std::runtime_error("index unavailable");
        }
    };
} // namespace detail

PAPILIO_EXPORT template <typename T, typename Context = format_context>
class accessor_traits : public detail::accessor_traits_base
{
public:
    using char_type = typename Context::char_type;
    using target_type = std::remove_cvref_t<T>;
    using accessor_type = accessor<target_type, Context>;

    using indexing_value_type = basic_indexing_value<char_type>;

    using string_view_type = std::basic_string_view<char_type>;

    using attribute_name_type = basic_attribute_name<char_type>;

    using index_type = typename indexing_value_type::index_type;

    using format_arg_type = basic_format_arg<Context>;

    [[nodiscard]]
    static constexpr bool has_integer_index() noexcept
    {
        return requires(T object, index_type i) {
            accessor_type::index(object, i);
        };
    }

    [[nodiscard]]
    static constexpr bool has_slice_index() noexcept
    {
        return requires(T object, slice s) {
            accessor_type::index(object, s);
        };
    }

    [[nodiscard]]
    static constexpr bool has_string_index() noexcept
    {
        return requires(T object, string_view_type s) {
            accessor_type::index(object, s);
        };
    }

    template <typename U>
    requires std::is_same_v<std::remove_cvref_t<U>, target_type>
    static format_arg_type index(U&& object, index_type i)
    {
        if constexpr(!has_integer_index())
        {
            integer_index_unavailable();
        }
        else
        {
            return format_arg_type(accessor_type::index(std::forward<U>(object), i));
        }
    }

    template <typename U>
    requires std::is_same_v<std::remove_cvref_t<U>, target_type>
    static format_arg_type index(U&& object, slice s)
    {
        if constexpr(!has_slice_index())
        {
            slice_index_unavailable();
        }
        else
        {
            return format_arg_type(accessor_type::index(std::forward<U>(object), s));
        }
    }

    template <typename U>
    requires std::is_same_v<std::remove_cvref_t<U>, target_type>
    static format_arg_type index(U&& object, string_view_type str)
    {
        if constexpr(!has_string_index())
        {
            string_index_unavailable();
        }
        else
        {
            return format_arg_type(accessor_type::index(std::forward<U>(object), str));
        }
    }

    [[nodiscard]]
    static constexpr bool has_attribute() noexcept
    {
        return requires(T object, const attribute_name_type& attr) {
            accessor_type::attribute(object, attr);
        };
    }

    template <typename U>
    requires std::is_same_v<std::remove_cvref_t<U>, target_type>
    static format_arg_type attribute(U&& object, const attribute_name_type& attr)
    {
        if constexpr(has_attribute())
        {
            return format_arg_type(accessor_type::attribute(std::forward<U>(object), attr));
        }
        else
        {
            throw_invalid_attribute(attr);
        }
    }

    template <typename U>
    requires std::is_same_v<std::remove_cvref_t<U>, target_type>
    static format_arg_type access(U&& object, const indexing_value_type& idx)
    {
        return idx.visit(
            [&](const auto& i) -> format_arg_type
            { return index(std::forward<U>(object), i); }
        );
    }

    template <typename U>
    requires std::is_same_v<std::remove_cvref_t<U>, target_type>
    static format_arg_type access(U&& object, const attribute_name_type& attr)
    {
        return attribute(std::forward<U>(object), attr);
    }
};
} // namespace papilio

#include "access/string.hpp"
#include "access/tuple.hpp"
#include "access/range.hpp"
#include "access/misc.hpp"

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif
