#pragma once

#include <memory>
#include <utility>
#include "macros.hpp"


namespace papilio
{
    namespace detail
    {
        template <typename T>
        concept pointer_like_helper = requires(T ptr, std::size_t idx)
        {
            typename std::pointer_traits<T>::pointer;
            ptr[idx];
        };

        template <bool HasMemberType, typename T, typename Deleter>
        struct deleter_traits_helper;
        template <typename T, typename Deleter>
        struct deleter_traits_helper<false, T, Deleter>
        {
            using pointer = T*;
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
        };

        template <typename T, typename Deleter>
        struct deleter_traits
        {
            using pointer = typename deleter_traits_helper<
                has_member_pointer_type<Deleter>(),
                T, Deleter
            >::pointer;
        };

        template <typename U, typename Pointer, typename Element>
        concept optional_arr_ptr_helper =
            std::is_same_v<Pointer, Element*> &&
            std::is_pointer_v<U> &&
            std::is_convertible_v<std::remove_pointer_t<U>(*)[], Element(*)[]>;

        template <typename U, typename Pointer, typename Element>
        concept optional_arr_ptr_acceptable =
            std::is_same_v<U, Pointer> ||
            std::is_same_v<U, std::nullptr_t> ||
            optional_arr_ptr_helper<U, Pointer, Element>;
    }

    template <typename T>
    concept pointer_like =
        (requires(T ptr) { *ptr; ptr.operator->(); } || requires(T ptr, std::size_t i) { ptr[i]; })
        && requires(T ptr) { static_cast<bool>(ptr); };

    // Smart pointer that owns an optional ownership of another object.
    // It can acts like a unique_ptr or a raw pointer.
    template <typename T, typename Deleter = std::default_delete<T>>
    class optional_ptr
    {
    public:
        using pointer = typename detail::deleter_traits<T, Deleter>::pointer;
        using element_type = T;
        using deleter_type = Deleter;

        optional_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>)
            : m_ptr(), m_del(), m_has_ownership(false) {}
        optional_ptr(std::nullptr_t) noexcept {}
        template <typename D>
        optional_ptr(const optional_ptr<T, D>& other)
            : m_ptr(other.m_ptr), m_has_ownership(false) {}
        optional_ptr(optional_ptr&& other) noexcept
            : m_ptr(std::exchange(other.m_ptr, pointer())),
            m_has_ownership(std::exchange(other.m_has_ownership, false)) {}
        explicit optional_ptr(pointer ptr, bool ownership = true) noexcept
            : m_ptr(std::move(ptr)), m_has_ownership(ownership) {}
        template <typename D>
        optional_ptr(std::unique_ptr<T, D>&& ptr) noexcept
            : m_ptr(ptr.release())
        {
            m_has_ownership = !m_ptr;
        }

        ~optional_ptr()
        {
            reset(nullptr);
        }

        optional_ptr& operator=(const optional_ptr& other)
        {
            reset(other.get());
            return *this;
        }

        template <typename U, typename D>
        bool operator==(const optional_ptr<U, D>& rhs) const
        {
            return get() == rhs.get();
        }
        friend bool operator==(const optional_ptr& lhs, std::nullptr_t) noexcept
        {
            return !lhs;
        }
        friend bool operator==(std::nullptr_t, const optional_ptr& rhs) noexcept
        {
            return !rhs;
        }

        // modifiers

        void reset(pointer ptr, bool ownership = true) noexcept
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
                m_del(get());
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
        void swap(optional_ptr<T, D>& other) noexcept
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
            return (bool)m_ptr;
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

        static optional_ptr pointer_to(element_type& val)
        {
            return optional_ptr(std::addressof(val), false);
        }

    private:
        pointer m_ptr;
        PAPILIO_NO_UNIQUE_ADDRESS Deleter m_del{};
        bool m_has_ownership = true;
    };

    template <typename T, typename Deleter>
    class optional_ptr<T[], Deleter>
    {
    public:
        using pointer = typename detail::deleter_traits<T, Deleter>::pointer;
        using element_type = T;
        using deleter_type = Deleter;

        optional_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>)
            : m_ptr(), m_del(), m_has_ownership(false) {}
        optional_ptr(std::nullptr_t) noexcept {}
        template <typename D>
        optional_ptr(const optional_ptr<T, D>& other)
            : m_ptr(other.m_ptr), m_has_ownership(false) {}
        optional_ptr(optional_ptr&& other) noexcept
            : m_ptr(std::exchange(other.m_ptr, pointer())),
            m_has_ownership(std::exchange(other.m_has_ownership, false)) {}
        template <typename U> requires detail::optional_arr_ptr_acceptable<U, pointer, element_type>
        explicit optional_ptr(U ptr, bool ownership = true) noexcept
            : m_ptr(ptr), m_has_ownership(ownership) {}
        template <typename D>
        optional_ptr(std::unique_ptr<T, D>&& ptr) noexcept
            : m_ptr(ptr.release())
        {
            m_has_ownership = !m_ptr;
        }

        ~optional_ptr()
        {
            reset(nullptr);
        }

        optional_ptr& operator=(const optional_ptr& other)
        {
            reset(other.get());
            return *this;
        }

        template <typename U, typename D>
        bool operator==(const optional_ptr<U, D>& rhs) const
        {
            return get() == rhs.get();
        }
        friend bool operator==(const optional_ptr& lhs, std::nullptr_t) noexcept
        {
            return !lhs;
        }
        friend bool operator==(std::nullptr_t, const optional_ptr& rhs) noexcept
        {
            return !rhs;
        }

        // modifiers

        void reset(pointer ptr, bool ownership = true) noexcept
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
                m_del(get());
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
        void swap(optional_ptr<T, D>& other) noexcept
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
            return (bool)m_ptr;
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
        PAPILIO_NO_UNIQUE_ADDRESS Deleter m_del{};
        bool m_has_ownership = true;
    };

    template <typename T, typename Deleter>
    optional_ptr(std::unique_ptr<T, Deleter>&&) -> optional_ptr<T, Deleter>;
}
