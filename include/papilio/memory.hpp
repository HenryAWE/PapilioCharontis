#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include "macros.hpp"


namespace papilio
{
    namespace detail
    {
        template <typename T>
        struct make_independent_proxy
        {
            using reference = std::add_lvalue_reference_t<T>;

            reference ref;

            [[nodiscard]]
            reference get() const noexcept
            {
                return ref;
            }
        };
    }
    struct independent_t
    {
        template <typename T>
        using proxy = detail::make_independent_proxy<T>;

        template <typename T>
        [[nodiscard]]
        constexpr proxy<T> operator()(T&& v) const noexcept
        {
            return proxy<T>(v);
        }
    };
    inline constexpr independent_t independent{};

    namespace detail
    {
        template <typename T>
        concept cp_is_empty = !std::is_final_v<T> && std::is_empty_v<T>;

        template <typename T1, typename T2>
        inline consteval int get_cp_impl_id() noexcept
        {
            using namespace std;
            if constexpr(!cp_is_empty<T1> && !cp_is_empty<T2>)
                return 0; // normal pair
            else if constexpr(cp_is_empty<T1> && !cp_is_empty<T2>)
                return 1; // only T1 is empty
            else if constexpr(!cp_is_empty<T1> && cp_is_empty<T2>)
                return 2; // only T2 is empty
            else
            {
                // both are empty
                if constexpr(!is_same_v<T1, T2>)
                    return 3; // T1 != T2
                else
                    return 1; // T1 == T2, fallback to implementation for only T1 is empty
            }
        }

        template <typename T1, typename T2, int Id>
        class compressed_pair_impl;

        // normal pair
        template <typename T1, typename T2>
        class compressed_pair_impl<T1, T2, 0>
        {
        public:
            using first_type = T1;
            using second_type = T2;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
            constexpr compressed_pair_impl(compressed_pair_impl&&) = default;
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : m_first(v1), m_second(v2) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : m_first(std::forward<U1>(v1)), m_second(std::forward<U2>(v2)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return m_first; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return m_first; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return m_second; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return m_second; }

            constexpr void swap(compressed_pair_impl& other)
                noexcept(std::is_nothrow_swappable_v<T1>&& std::is_nothrow_swappable_v<T2>)
            {
                using std::swap;
                swap(m_first, other.first());
                swap(m_second, other.second());
            }

        private:
            first_type m_first;
            second_type m_second;
        };

        // T1 is empty
        template <typename T1, typename T2>
        class compressed_pair_impl<T1, T2, 1> : private std::remove_cv_t<T1>
        {
        public:
            using first_type = T1;
            using second_type = T2;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
            constexpr compressed_pair_impl(compressed_pair_impl&&) = default;
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : T1(v1), m_second(v2) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : T1(std::forward<U1>(v1)), m_second(std::forward<U2>(v2)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return *this; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return *this; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return m_second; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return m_second; }

            constexpr void swap(compressed_pair_impl& other)
                noexcept(std::is_nothrow_swappable_v<T2>)
            {
                using std::swap;
                swap(m_second, other.second());
            }

        private:
            second_type m_second;
        };

        // T2 is empty
        template <typename T1, typename T2>
        class compressed_pair_impl<T1, T2, 2> : private std::remove_cv_t<T2>
        {
        public:
            using first_type = T1;
            using second_type = T2;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
            constexpr compressed_pair_impl(compressed_pair_impl&&) = default;
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : T2(v2), m_first(v1) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : T2(std::forward<U1>(v2)), m_first(std::forward<U2>(v1)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return m_first; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return m_first; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return *this; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return *this; }

            constexpr void swap(compressed_pair_impl& other)
                noexcept(std::is_nothrow_swappable_v<T1>)
            {
                using std::swap;
                swap(m_first, other.first());
            }

        private:
            first_type m_first;
        };

        // T1 != T2, both are empty
        template <typename T1, typename T2>
        class compressed_pair_impl<T1, T2, 3> : private std::remove_cv_t<T1>, private std::remove_cv_t<T2>
        {
        public:
            using first_type = T1;
            using second_type = T2;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
            constexpr compressed_pair_impl(compressed_pair_impl&&) = default;
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : T1(v1), T2(v2) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : T1(std::forward<U1>(v1)), T2(std::forward<U2>(v2)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return *this; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return *this; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return *this; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return *this; }

            constexpr void swap(compressed_pair_impl& other) noexcept
            {
                // empty
            }
        };
    }

    template <typename T1, typename T2>
    class compressed_pair :
        public detail::compressed_pair_impl<T1, T2, detail::get_cp_impl_id<T1, T2>()>
    {
        using base = detail::compressed_pair_impl<T1, T2, detail::get_cp_impl_id<T1, T2>()>;
    public:
        using base::base;
    };

    template <std::size_t Capacity, std::size_t Align = alignof(std::max_align_t)>
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
        alignas(Align) std::byte m_data[Capacity]{};
    };
    template <std::size_t Align>
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

        template <typename Derived, typename Element>
        class optional_ptr_base
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
    }

    template <typename T>
    concept pointer_like =
        (requires(T ptr) { *ptr; ptr.operator->(); } || requires(T ptr, std::size_t i) { ptr[i]; })
        && requires(T ptr) { static_cast<bool>(ptr); };

    // Smart pointer that owns an optional ownership of another object.
    // It can acts like a unique_ptr or a raw pointer.
    template <typename T, typename Deleter = std::default_delete<T>>
    class optional_unique_ptr : public detail::optional_ptr_base<optional_unique_ptr<T, Deleter>, T>
    {
    public:
        using pointer = typename detail::deleter_traits<T, Deleter>::pointer;
        using element_type = T;
        using deleter_type = Deleter;

        optional_unique_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>)
            : m_ptr(), m_del(), m_has_ownership(false) {}
        optional_unique_ptr(std::nullptr_t) noexcept {}
        optional_unique_ptr(const optional_unique_ptr& other) noexcept
            : m_ptr(other.get()),
            m_del(other.get_deleter()),
            m_has_ownership(false) {}
        template <typename D>
        optional_unique_ptr(const optional_unique_ptr<T, D>& other)
            : m_ptr(other.m_ptr), m_has_ownership(false) {}
        optional_unique_ptr(optional_unique_ptr&& other) noexcept
            : m_ptr(std::exchange(other.m_ptr, pointer())),
            m_has_ownership(std::exchange(other.m_has_ownership, false)) {}
        explicit optional_unique_ptr(pointer ptr, bool ownership = true) noexcept
            : m_ptr(std::move(ptr)), m_has_ownership(ownership) {}
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

    private:
        pointer m_ptr;
        PAPILIO_NO_UNIQUE_ADDRESS Deleter m_del{};
        bool m_has_ownership = true;
    };

    template <typename T, typename Deleter>
    class optional_unique_ptr<T[], Deleter>
    {
    public:
        using pointer = typename detail::deleter_traits<T, Deleter>::pointer;
        using element_type = T;
        using deleter_type = Deleter;

        optional_unique_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>)
            : m_ptr(), m_del(), m_has_ownership(false) {}
        optional_unique_ptr(std::nullptr_t) noexcept {}
        optional_unique_ptr(const optional_unique_ptr& other) noexcept
            : m_ptr(other.get()),
            m_del(other.get_deleter()),
            m_has_ownership(false) {}
        template <typename D>
        optional_unique_ptr(const optional_unique_ptr<T, D>& other)
            : m_ptr(other.m_ptr), m_has_ownership(false) {}
        optional_unique_ptr(optional_unique_ptr&& other) noexcept
            : m_ptr(std::exchange(other.m_ptr, pointer())),
            m_has_ownership(std::exchange(other.m_has_ownership, false)) {}
        template <typename U> requires detail::optional_arr_ptr_acceptable<U, pointer, element_type>
        explicit optional_unique_ptr(U ptr, bool ownership = true) noexcept
            : m_ptr(ptr), m_has_ownership(ownership) {}
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
        bool m_has_ownership = true;
        PAPILIO_NO_UNIQUE_ADDRESS Deleter m_del{};
    };

    template <typename T, typename Deleter>
    optional_unique_ptr(std::unique_ptr<T, Deleter>&&) -> optional_unique_ptr<T, Deleter>;
}
