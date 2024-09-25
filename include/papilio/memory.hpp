/**
 * @file memory.hpp
 * @author HenryAWE
 * @brief Memory management utilities.
 */

#ifndef PAPILIO_MEMORY_HPP
#define PAPILIO_MEMORY_HPP

#pragma once

#include <cstddef>
#include <memory>
#include "utility.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
/**
 * @brief Aligned static storage
 *
 * @tparam Capacity Maximum size of storage in bytes
 * @tparam Align Alignment of storage
 */
PAPILIO_EXPORT template <
    std::size_t Capacity,
    std::size_t Align = alignof(std::max_align_t)>
class static_storage
{
public:
    constexpr static_storage() noexcept = default;
    static_storage(const static_storage&) = delete;

    [[nodiscard]]
    constexpr std::byte* data() noexcept
    {
        return m_data;
    }

    [[nodiscard]]
    constexpr const std::byte* data() const noexcept
    {
        return m_data;
    }

    [[nodiscard]]
    static constexpr std::size_t size() noexcept
    {
        return Capacity;
    }

private:
    alignas(Align) std::byte m_data[Capacity];
};

/**
 * @brief Specialization of `static_storage` for zero capacity.
 */
PAPILIO_EXPORT template <std::size_t Align>
class static_storage<0, Align>
{
public:
    [[nodiscard]]
    constexpr std::byte* data() noexcept
    {
        return nullptr;
    }

    [[nodiscard]]
    constexpr const std::byte* data() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]]
    static constexpr std::size_t size() noexcept
    {
        return 0;
    }
};

namespace detail
{
    template <bool HasMemberType, typename T, typename Deleter>
    struct deleter_traits_helper;

    template <typename T, typename Deleter>
    struct deleter_traits_helper<false, T, Deleter>
    {
        using pointer = std::add_pointer_t<T>;
    };

    template <typename T, typename Deleter>
    struct deleter_traits_helper<true, T, Deleter>
    {
        using pointer = typename Deleter::pointer;
    };

    template <typename Deleter>
    consteval bool has_member_pointer_type()
    {
        return requires() { typename Deleter::pointer; };
    }

    template <typename T, typename Deleter>
    struct deleter_traits
    {
        using pointer = typename deleter_traits_helper<
            has_member_pointer_type<Deleter>(),
            T,
            Deleter>::pointer;
    };

    template <typename U, typename Pointer, typename Element>
    concept optional_arr_ptr_helper =
        std::is_same_v<Pointer, Element*> &&
        std::is_pointer_v<U> &&
        std::is_convertible_v<std::remove_pointer_t<U> (*)[], Element (*)[]>;

    template <typename U, typename Pointer, typename Element>
    concept optional_arr_ptr_acceptable =
        std::is_same_v<U, Pointer> ||
        std::is_same_v<U, std::nullptr_t> ||
        optional_arr_ptr_helper<U, Pointer, Element>;

    template <typename Derived, typename Element>
    class optional_ptr_base // non-void type
    {
    public:
        static Derived pointer_to(Element& val)
        {
            return Derived(std::addressof(val), false);
        }
    };

    template <typename Derived>
    class optional_ptr_base<Derived, void>
    {
    public:
        // empty
    };
} // namespace detail

/**
 * @brief Smart pointer that owns an optional ownership of another object.
 *
 * The `optional_unique_ptr` acts like a `unique_ptr` or a raw pointer depending on its ownership state.
 *
 * @tparam T Element type
 * @tparam Deleter Deleter
 */
PAPILIO_EXPORT template <
    typename T,
    typename Deleter = std::default_delete<T>>
class optional_unique_ptr : public detail::optional_ptr_base<optional_unique_ptr<T, Deleter>, T>
{
public:
    using pointer = typename detail::deleter_traits<T, Deleter>::pointer;
    using element_type = T;
    using deleter_type = Deleter;

    optional_unique_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : m_ptr(), m_del(), m_has_ownership(false) {}

    optional_unique_ptr(std::nullptr_t) noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : optional_unique_ptr() {}

    optional_unique_ptr(const optional_unique_ptr& other) noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : m_ptr(other.get()),
          m_del(other.get_deleter()),
          m_has_ownership(false) {}

    template <typename D>
    optional_unique_ptr(const optional_unique_ptr<T, D>& other)
        : m_ptr(other.m_ptr),
          m_has_ownership(false)
    {}

    optional_unique_ptr(optional_unique_ptr&& other) noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : m_ptr(std::exchange(other.m_ptr, pointer())),
          m_del(),
          m_has_ownership(std::exchange(other.m_has_ownership, false)) {}

    optional_unique_ptr(pointer ptr, bool ownership) noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : m_ptr(std::move(ptr)),
          m_del(),
          m_has_ownership(ownership) {}

    optional_unique_ptr(independent_t, pointer ptr) noexcept
        : m_ptr(std::move(ptr)),
          m_del(),
          m_has_ownership(true) {}

    template <typename D>
    optional_unique_ptr(std::unique_ptr<T, D>&& ptr) noexcept
        : m_ptr(ptr.release())
    {
        m_has_ownership = m_ptr;
    }

    ~optional_unique_ptr()
    {
        reset(nullptr);
    }

    optional_unique_ptr& operator=(const optional_unique_ptr& other)
    {
        reset(other.get());
        return *this;
    }

    template <typename U, typename D>
    bool operator==(const optional_unique_ptr<U, D>& rhs) const
    {
        return get() == rhs.get();
    }

    friend bool operator==(const optional_unique_ptr& lhs, std::nullptr_t) noexcept
    {
        return !lhs;
    }

    friend bool operator==(std::nullptr_t, const optional_unique_ptr& rhs) noexcept
    {
        return !rhs;
    }

    // modifiers

    void reset(pointer ptr, bool ownership) noexcept
    {
        reset(nullptr);
        if(ptr)
        {
            m_ptr = std::move(ptr);
            m_has_ownership = ownership;
        }
    }

    void reset(independent_t, pointer ptr) noexcept
    {
        reset(std::move(ptr), true);
    }

    template <typename D>
    void reset(std::unique_ptr<T, D>&& ptr) noexcept
    {
        reset(nullptr);
        if(ptr)
        {
            m_ptr = ptr.release();
            m_has_ownership = true;
        }
    }

    void reset(std::nullptr_t) noexcept
    {
        if(has_ownership())
        {
            PAPILIO_ASSERT(static_cast<bool>(*this));
            get_deleter()(get());
        }
        m_ptr = pointer();
        m_has_ownership = false;
    }

    void reset() noexcept
    {
        reset(nullptr);
    }

    pointer release() noexcept
    {
        m_has_ownership = false;
        return std::exchange(m_ptr, pointer());
    }

    template <typename D>
    void swap(optional_unique_ptr<T, D>& other) noexcept
    {
        using std::swap;
        swap(m_ptr, other.m_ptr);
        swap(m_has_ownership, other.m_has_ownership);
    }

    // observers

    [[nodiscard]]
    pointer get() const noexcept
    {
        return m_ptr;
    }

    Deleter& get_deleter() noexcept
    {
        return m_del;
    }

    const Deleter& get_deleter() const noexcept
    {
        return m_del;
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_ptr);
    }

    [[nodiscard]]
    bool has_ownership() const noexcept
    {
        return m_has_ownership;
    }

    std::add_lvalue_reference_t<T> operator*() const noexcept(noexcept(*std::declval<pointer>()))
    {
        return *m_ptr;
    }

    pointer operator->() const noexcept
    {
        return m_ptr;
    }

private:
    pointer m_ptr;
    PAPILIO_NO_UNIQUE_ADDRESS Deleter m_del;
    bool m_has_ownership;
};

PAPILIO_EXPORT template <
    typename T,
    typename Deleter>
class optional_unique_ptr<T[], Deleter>
{
public:
    using pointer = typename detail::deleter_traits<T, Deleter>::pointer;
    using element_type = T;
    using deleter_type = Deleter;

    optional_unique_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : m_ptr(), m_del(), m_has_ownership(false) {}

    optional_unique_ptr(std::nullptr_t) noexcept(std::is_nothrow_default_constructible_v<Deleter>)
        : optional_unique_ptr() {}

    optional_unique_ptr(const optional_unique_ptr& other) noexcept
        : m_ptr(other.get()),
          m_del(other.get_deleter()),
          m_has_ownership(false) {}

    template <typename D>
    optional_unique_ptr(const optional_unique_ptr<T, D>& other)
        : m_ptr(other.m_ptr), m_del(), m_has_ownership(false)
    {}

    optional_unique_ptr(optional_unique_ptr&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, pointer())),
          m_del(),
          m_has_ownership(std::exchange(other.m_has_ownership, false)) {}

    template <typename U>
    requires detail::optional_arr_ptr_acceptable<U, pointer, element_type>
    optional_unique_ptr(U ptr, bool ownership) noexcept
        : m_ptr(ptr), m_del(), m_has_ownership(ownership)
    {}

    template <typename D>
    optional_unique_ptr(std::unique_ptr<T, D>&& ptr) noexcept
        : m_ptr(ptr.release())
    {
        m_has_ownership = !m_ptr;
    }

    ~optional_unique_ptr()
    {
        reset(nullptr);
    }

    optional_unique_ptr& operator=(const optional_unique_ptr& other)
    {
        reset(other.get());
        return *this;
    }

    template <typename U, typename D>
    bool operator==(const optional_unique_ptr<U, D>& rhs) const
    {
        return get() == rhs.get();
    }

    friend bool operator==(const optional_unique_ptr& lhs, std::nullptr_t) noexcept
    {
        return !lhs;
    }

    friend bool operator==(std::nullptr_t, const optional_unique_ptr& rhs) noexcept
    {
        return !rhs;
    }

    // modifiers

    void reset(pointer ptr, bool ownership) noexcept
    {
        reset(nullptr);
        if(ptr)
        {
            m_ptr = std::move(ptr);
            m_has_ownership = ownership;
        }
    }

    template <typename D>
    void reset(std::unique_ptr<T, D>&& ptr) noexcept
    {
        reset(nullptr);
        if(ptr)
        {
            m_ptr = ptr.release();
            m_has_ownership = true;
        }
    }

    void reset(std::nullptr_t) noexcept
    {
        if(has_ownership())
        {
            PAPILIO_ASSERT(static_cast<bool>(*this));
            m_del(get());
        }
        m_ptr = pointer();
        m_has_ownership = false;
    }

    void reset() noexcept
    {
        reset(nullptr);
    }

    pointer release() noexcept
    {
        m_has_ownership = false;
        return std::exchange(m_ptr, pointer());
    }

    template <typename D>
    void swap(optional_unique_ptr<T, D>& other) noexcept
    {
        using std::swap;
        swap(m_ptr, other.m_ptr);
        swap(m_has_ownership, other.m_has_ownership);
    }

    // observers

    [[nodiscard]]
    pointer get() const noexcept
    {
        return m_ptr;
    }

    Deleter& get_deleter() noexcept
    {
        return m_del;
    }

    const Deleter& get_deleter() const noexcept
    {
        return m_del;
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_ptr);
    }

    [[nodiscard]]
    bool has_ownership() const noexcept
    {
        return m_has_ownership;
    }

    T& operator[](std::size_t i) const
    {
        return get()[i];
    }

private:
    pointer m_ptr;
    PAPILIO_NO_UNIQUE_ADDRESS Deleter m_del;
    bool m_has_ownership = true;
};

PAPILIO_EXPORT template <typename T, typename Deleter>
optional_unique_ptr(std::unique_ptr<T, Deleter>&&) -> optional_unique_ptr<T, Deleter>;

PAPILIO_EXPORT template <typename T, typename... Args>
requires(!std::is_array_v<T>)
optional_unique_ptr<T> make_optional_unique(Args&&... args)
{
    return optional_unique_ptr<T>(new T(std::forward<Args>(args)...), true);
}

PAPILIO_EXPORT template <typename T, typename... Args>
requires(std::is_array_v<T> && std::extent_v<T> == 0)
optional_unique_ptr<T> make_optional_unique(std::size_t n)
{
    using element_type = std::remove_extent_t<T>;
    return optional_unique_ptr<T>(new element_type[n](), true);
}

PAPILIO_EXPORT template <typename T, typename... Args>
requires(std::is_array_v<T> && std::extent_v<T> != 0)
optional_unique_ptr<T> make_optional_unique(std::size_t n) = delete;
} // namespace papilio

#include "detail/suffix.hpp"

#endif
