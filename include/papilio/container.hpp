#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <array>
#include <utility>
#include <numeric>
#include <stdexcept>
#include "macros.hpp"
#include "memory.hpp"

namespace papilio
{
namespace detail
{
    class small_vector_impl_base
    {
    public:
        using size_type = std::size_t;

        [[noreturn]]
        static void throw_out_of_range();
        [[noreturn]]
        static void throw_length_error();

    protected:
        [[nodiscard]]
        static size_type calc_mem_size(size_type current, size_type required) noexcept
        {
            PAPILIO_ASSERT(current < required);
            if(current <= 1)
                current = 2;
            while(current < required)
                current += current / 2;
            return current;
        }
    };
} // namespace detail

template <typename T, typename Allocator = std::allocator<T>>
class small_vector_base : public detail::small_vector_impl_base
{
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // element access

    reference at(size_type i)
    {
        if(i < size())
            return data()[i];
        this->throw_out_of_range();
    }

    const_reference at(size_type i) const
    {
        if(i < size())
            return data()[i];
        this->throw_out_of_range();
    }

    // clang-format off

    [[nodiscard]]
    reference operator[](size_type i) noexcept
    {
        return data()[i];
    }

    [[nodiscard]]
    const_reference operator[](size_type i) const noexcept
    {
        return data()[i];
    }

    // clang-format on

    [[nodiscard]]
    reference front() noexcept
    {
        PAPILIO_ASSERT(!empty());
        return *begin();
    }

    [[nodiscard]]
    const_reference front() const noexcept
    {
        PAPILIO_ASSERT(!empty());
        return *begin();
    }

    [[nodiscard]]
    reference back() noexcept
    {
        PAPILIO_ASSERT(!empty());
        return *(end() - 1);
    }

    [[nodiscard]]
    const_reference back() const noexcept
    {
        PAPILIO_ASSERT(!empty());
        return *(end() - 1);
    }

    [[nodiscard]]
    pointer data() noexcept
    {
        return m_p_begin;
    }

    [[nodiscard]]
    const_pointer data() const noexcept
    {
        return m_p_begin;
    }

    // iterators

    iterator begin() noexcept
    {
        return m_p_begin;
    }

    iterator end() noexcept
    {
        return m_p_end;
    }

    const_iterator begin() const noexcept
    {
        return m_p_begin;
    }

    const_iterator end() const noexcept
    {
        return m_p_end;
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    const_iterator cend() const noexcept
    {
        return end();
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    // capacity

    [[nodiscard]]
    bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]]
    size_type size() const noexcept
    {
        return m_p_end - m_p_begin;
    }

    [[nodiscard]]
    size_type max_size() const noexcept
    {
        return std::numeric_limits<difference_type>::max();
    }

    [[nodiscard]]
    size_type capacity() const noexcept
    {
        return m_p_capacity - m_p_begin;
    }

protected:
    pointer m_p_begin;
    pointer m_p_end;
    pointer m_p_capacity;

    // construct an empty container
    small_vector_base() noexcept
        : m_p_begin(), m_p_end(), m_p_capacity() {}

    void swap_ptrs(small_vector_base& other) noexcept
    {
        using std::swap;

        swap(m_p_begin, other.m_p_begin);
        swap(m_p_end, other.m_p_end);
        swap(m_p_capacity, other.m_p_capacity);
    }
};

template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
class small_vector : public small_vector_base<T, Allocator>
{
    using base = small_vector_base<T, Allocator>;

public:
    static_assert(std::is_object_v<T>, "Container of non-object type is invalid");

    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = base::pointer;
    using const_pointer = base::const_pointer;
    using iterator = base::iterator;
    using const_iterator = base::const_iterator;
    using reverse_iterator = base::reverse_iterator;
    using const_reverse_iterator = base::const_reverse_iterator;

    small_vector() noexcept(std::is_nothrow_default_constructible_v<Allocator>)
        : base()
    {
        set_ptrs(getbuf(), static_size());
    }

    explicit small_vector(const Allocator& alloc) noexcept
        : base(),
          m_data(std::piecewise_construct, std::forward_as_tuple(), std::forward_as_tuple(alloc))
    {
        set_ptrs(getbuf(), static_size());
    }

    small_vector(const small_vector& other)
        : small_vector()
    {
        reserve(other.size());
        for(size_type i = 0; i < other.size(); ++i)
        {
            emplace_back_impl(other[i]);
        }
    }

    small_vector(small_vector&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
        : base()
    {
        if(other.dynamic_allocated())
        {
            pointer other_buf = other.getbuf();
            m_p_begin = std::exchange(other.m_p_begin, other_buf);
            m_p_end = std::exchange(other.m_p_end, other_buf);
            m_p_capacity = std::exchange(other.m_p_capacity, other_buf + static_size());
        }
        else [[likely]]
        {
            set_ptrs(getbuf(), static_size());

            PAPILIO_ASSERT(other.size() <= this->capacity());
            for(auto&& i : other)
            {
                emplace_back_impl(std::move(i));
            }
            other.clear();
        }
    }

    template <typename Iterator>
    small_vector(Iterator first, Iterator last, const Allocator& alloc = Allocator())
        : small_vector(alloc)
    {
        assign(first, last);
    }

    small_vector(std::initializer_list<value_type> il, const Allocator& alloc = Allocator())
        : small_vector(alloc)
    {
        assign(il.begin(), il.end());
    }

    ~small_vector() noexcept
    {
        clear();
        free_mem();
    }

    small_vector& operator=(const small_vector& rhs)
    {
        if(this == &rhs)
            return *this;
        assign(rhs.begin(), rhs.end());
        return *this;
    }

    small_vector& operator=(small_vector&& rhs) noexcept(std::is_nothrow_move_constructible_v<small_vector>)
    {
        small_vector tmp(std::move(rhs));
        swap(tmp);
        return *this;
    }

    template <typename Iterator>
    void assign(Iterator first, Iterator last)
    {
        clear();
        constexpr bool random_access = std::is_convertible_v<
            typename std::iterator_traits<Iterator>::iterator_category,
            std::random_access_iterator_tag>;
        if constexpr(random_access)
        {
            reserve(std::distance(first, last));
            for(auto it = first; it != last; ++it)
            {
                emplace_back_impl(*it);
            }
        }
        else
        {
            for(auto it = first; it != last; ++it)
            {
                emplace_back(*it);
            }
        }
    }

    void assign(std::initializer_list<value_type> il)
    {
        assign(il.begin(), il.end());
    }

    allocator_type get_allocator() const noexcept
    {
        return getal();
    }

    // capacity

    [[nodiscard]]
    bool dynamic_allocated() const noexcept
    {
        return m_p_begin != getbuf();
    }

    [[nodiscard]]
    static constexpr size_type static_size() noexcept
    {
        return N;
    }

    void reserve(size_type n)
    {
        if(n <= this->capacity())
            return;
        grow_mem(n);
    }

    void shrink_to_fit()
    {
        shrink_mem();
    }

    // modifiers

    void clear() noexcept
    {
        destroy_all();
        m_p_end = m_p_begin; // set size to 0
    }

    void push_back(const value_type& val)
    {
        emplace_back(val);
    }

    void push_back(value_type&& val)
    {
        emplace_back(std::move(val));
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        if(m_p_end == m_p_capacity)
        {
            size_type current = this->size();
            reserve(this->calc_mem_size(current, current + 1));
        }
        emplace_back_impl(std::forward<Args>(args)...);
    }

    void pop_back() noexcept
    {
        PAPILIO_ASSERT(!this->empty());
        std::allocator_traits<Allocator>::destroy(
            getal(),
            m_p_end - 1
        );
        --m_p_end;
    }

    void resize(size_type count, const value_type& val)
    {
        if(count < this->size())
        {
            while(this->size() != count)
                pop_back();
        }
        else if(count > this->size())
        {
            reserve(count);
            while(this->size() != count)
                emplace_back_impl(val);
        }
    }

    void resize(size_type count) requires std::is_default_constructible_v<value_type>
    {
        resize(count, value_type());
    }

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(push)
#    pragma warning(disable : 26800) // lifetime.1
#endif

    void swap(small_vector& other) noexcept
    {
        using std::swap;

        if(dynamic_allocated() && other.dynamic_allocated())
        {
            // both are dynamic allocated
            swap_ptrs(other);
            if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value)
                swap(getal(), other.getal());
        }
        else if(!dynamic_allocated() && other.dynamic_allocated())
        {
            swap_static_to_dyn(other);
        }
        else if(!other.dynamic_allocated() && dynamic_allocated())
        {
            other.swap_static_to_dyn(*this);
        }
        else
        {
            // both are using static storage
            PAPILIO_ASSERT(!dynamic_allocated() && !other.dynamic_allocated());

            size_type tmp_size_1 = this->size();
            size_type tmp_size_2 = other.size();
            for(size_type i = 0; i < std::max(tmp_size_1, tmp_size_2); ++i)
            {
                if(i < this->size() && i >= other.size())
                {
                    // this->size() > other.size()
                    // copy elements to the other one
                    std::construct_at(
                        other.m_p_begin + i,
                        std::move(*(m_p_begin + i))
                    );
                    std::destroy_at(getbuf() + i);
                }
                else if(i >= this->size() && i < other.size())
                {
                    // this->size() < other.size()
                    // copy elements from the other one
                    std::construct_at(
                        m_p_begin + i,
                        std::move(*(other.m_p_begin + i))
                    );
                    std::destroy_at(other.m_p_begin + i);
                }
                else
                {
                    swap(*(m_p_begin + i), *(other.m_p_begin + i));
                }
            }

            // set correct size values
            m_p_end = m_p_begin + tmp_size_2;
            other.m_p_end = other.m_p_begin + tmp_size_1;
            if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value)
                swap(getal(), other.getal());
        }
    }

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(pop)
#endif

private:
    using base::m_p_begin;
    using base::m_p_capacity;
    using base::m_p_end;
    compressed_pair<
        static_storage<sizeof(value_type) * N, alignof(value_type)>,
        allocator_type>
        m_data;

    value_type* getbuf() noexcept
    {
        return reinterpret_cast<value_type*>(m_data.first().data());
    }

    const value_type* getbuf() const noexcept
    {
        return reinterpret_cast<const value_type*>(m_data.first().data());
    }

    // get reference of the allocator
    allocator_type& getal() noexcept
    {
        return m_data.second();
    }

    void set_ptrs(pointer p_begin, size_type capacity_offset) noexcept
    {
        m_p_begin = p_begin;
        m_p_end = p_begin;
        m_p_capacity = p_begin + capacity_offset;
    }

    void set_ptrs(pointer p_begin, size_type size_offset, size_type capacity_offset) noexcept
    {
        PAPILIO_ASSERT(size_offset <= capacity_offset);

        m_p_begin = p_begin;
        m_p_end = p_begin + size_offset;
        m_p_capacity = p_begin + capacity_offset;
    }

    void set_ptrs(pointer p_begin, pointer p_end, pointer p_capacity) noexcept
    {
        PAPILIO_ASSERT(m_p_begin <= m_p_end);
        PAPILIO_ASSERT(m_p_end <= m_p_capacity);

        m_p_begin = p_begin;
        m_p_end = p_end;
        m_p_capacity = p_capacity;
    }

    using base::swap_ptrs;

    // Note: This function assumes that the container has enough memory.
    template <typename... Args>
    void emplace_back_impl(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        PAPILIO_ASSERT(m_p_end < m_p_capacity);
        if(!dynamic_allocated()) [[likely]]
        {
            std::construct_at(
                static_cast<value_type*>(m_p_end),
                std::forward<Args>(args)...
            );
        }
        else
        {
            std::allocator_traits<Allocator>::construct(
                getal(),
                m_p_end,
                std::forward<Args>(args)...
            );
        }
        ++m_p_end;
    }

    // Note: This function doesn't automatically reset the pointers.
    void free_mem() noexcept
    {
        if(!dynamic_allocated())
            return;
        std::allocator_traits<Allocator>::deallocate(
            getal(),
            m_p_begin,
            this->capacity()
        );
    }

    // Note: This function doesn't automatically reset the pointers.
    void destroy_all() noexcept
    {
        if(!dynamic_allocated())
        {
            std::destroy_n(m_p_begin, this->size());
        }
        else
        {
            for(pointer i = m_p_begin; i < m_p_end; ++i)
            {
                std::allocator_traits<Allocator>::destroy(
                    getal(),
                    i
                );
            }
        }
    }

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(push)
#    pragma warning(disable : 26800) // lifetime.1
#endif

    void shrink_mem()
    {
        if(this->size() <= static_size())
        {
            if(!dynamic_allocated())
            {
                PAPILIO_ASSERT(this->capacity() == static_size());
                return;
            }

            // move from dynamic allocated memory to static storage
            pointer tmp_ptr = m_p_begin;
            size_type tmp_size = this->size();
            size_type tmp_capacity = this->capacity();
            m_p_begin = getbuf();

            for(size_type i = 0; i < tmp_size; ++i)
            {
                std::construct_at(
                    m_p_begin + i,
                    std::move(*(tmp_ptr + i))
                );
                std::allocator_traits<Allocator>::destroy(
                    getal(),
                    tmp_ptr + i
                );
            }
            m_p_end = m_p_begin + tmp_size;

            std::allocator_traits<Allocator>::deallocate(
                getal(),
                tmp_ptr,
                tmp_capacity
            );

            m_p_capacity = m_p_begin + static_size();
        }
    }

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(pop)
#endif

    void grow_mem(size_type new_cap)
    {
        PAPILIO_ASSERT(this->capacity() < new_cap);
        if(new_cap > this->max_size())
            this->throw_length_error();

        pointer new_mem = getal().allocate(new_cap);
        size_type i = 0;
        try
        {
            size_type tmp_size = this->size();
            for(i = 0; i < tmp_size; ++i)
            {
                std::allocator_traits<Allocator>::construct(
                    getal(),
                    new_mem + i,
                    std::move(*(m_p_begin + i))
                );
            }
            destroy_all(); // size is unchanged
            free_mem();
            set_ptrs(new_mem, tmp_size, new_cap);
        }
        catch(...)
        {
            for(size_type j = 0; j < i; ++j)
            {
                std::allocator_traits<Allocator>::destroy(
                    getal(),
                    new_mem + j
                );
            }
            std::allocator_traits<Allocator>::deallocate(
                getal(),
                new_mem,
                new_cap
            );
            throw;
        }
    }

    // Swap data from a small_vector using static storage to a small_vector using dynamic allocated memory.
    // Note: This function assumes that current small_vector is using the static storage.
    void swap_static_to_dyn(small_vector& other)
    {
        using std::swap;

        PAPILIO_ASSERT(!dynamic_allocated());

        pointer tmp_p_begin = other.m_p_begin;
        pointer tmp_p_end = other.m_p_end;
        pointer tmp_p_capacity = other.m_p_capacity;
        other.set_ptrs(other.getbuf(), static_size());

        for(size_type i = 0; i < this->size(); ++i)
        {
            other.emplace_back_impl(
                std::move(*(m_p_begin + i))
            );
        }
        destroy_all();

        set_ptrs(tmp_p_begin, tmp_p_end, tmp_p_capacity);
        if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value)
            swap(getal(), other.getal());
    }
};

namespace detail
{
    class fixed_vector_impl_base
    {
    public:
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        [[noreturn]]
        static void throw_out_of_range();
        [[noreturn]]
        static void throw_length_error();
    };
} // namespace detail

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(push)
#    pragma warning(disable : 26495)
#endif

template <typename T, std::size_t Capacity>
class fixed_vector : public detail::fixed_vector_impl_base
{
public:
    static_assert(std::is_object_v<T>, "Container of non-object type is invalid");

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;

    fixed_vector() noexcept
        : m_size(0) {}

    fixed_vector(const fixed_vector& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        for(size_type i = 0; i < other.size(); ++i)
        {
            emplace_back(other[i]);
        }
    }

    fixed_vector(fixed_vector&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        for(size_type i = 0; i < other.size(); ++i)
        {
            emplace_back(std::move(other[i]));
        }
    }

    fixed_vector(size_type count, const T& value)
    {
        for(size_type i = 0; i < count; ++i)
        {
            emplace_back(value);
        }
    }

    ~fixed_vector() noexcept
    {
        clear();
    }

    reference at(size_type pos)
    {
        [[unlikely]] if(pos >= size())
            throw_out_of_range();
        return data()[pos];
    }

    const_reference at(size_type pos) const
    {
        [[unlikely]] if(pos >= size())
            throw_out_of_range();
        return data()[pos];
    }

    // clang-format off

    [[nodiscard]]
    reference operator[](size_type pos) noexcept
    {
        return data()[pos];
    }

    [[nodiscard]]
    const_reference operator[](size_type pos) const noexcept
    {
        return data()[pos];
    }

    // clang-format on

    [[nodiscard]]
    reference front() noexcept
    {
        return *begin();
    }

    [[nodiscard]]
    const_reference front() const noexcept
    {
        return *begin();
    }

    [[nodiscard]]
    reference back() noexcept
    {
        return *(end() - 1);
    }

    [[nodiscard]]
    const_reference back() const noexcept
    {
        return *(end() - 1);
    }

    pointer data() noexcept
    {
        return static_cast<pointer>(getbuf());
    }

    const_pointer data() const noexcept
    {
        return static_cast<const_pointer>(getbuf());
    }

    // iterators

    iterator begin() noexcept
    {
        return data();
    }

    iterator end() noexcept
    {
        return data() + size();
    }

    const_iterator begin() const noexcept
    {
        return data();
    }

    const_iterator end() const noexcept
    {
        return data() + size();
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    const_iterator cend() const noexcept
    {
        return end();
    }

    // capacity

    [[nodiscard]]
    bool empty() const noexcept
    {
        return m_size == 0;
    }

    [[nodiscard]]
    size_type size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]]
    static constexpr size_type max_size() noexcept
    {
        return Capacity;
    }

    [[nodiscard]]
    size_type capacity() const noexcept
    {
        return max_size();
    }

    // modifiers

    void clear() noexcept
    {
        std::destroy_n(data(), size());
        m_size = 0;
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        PAPILIO_ASSERT(pos >= cbegin());
        PAPILIO_ASSERT(pos <= cend());

        if(pos == cend())
        {
            return std::addressof(emplace_back(std::forward<Args>(args)...));
        }
        else if(m_size == capacity())
        {
            throw_length_error();
        }

        emplace_back(std::move(back()));
        if(size() > 2)
        {
            for(iterator it = end() - 2; it != pos; --it)
            {
                *it = std::move(*std::prev(it));
            }
        }

        value_type tmp(std::forward<Args>(args)...);
        iterator nonconst_pos = const_cast<iterator>(pos);
        *nonconst_pos = std::move(tmp);

        return nonconst_pos;
    }

    iterator insert(const_iterator pos, const T& val)
    {
        return emplace(pos, val);
    }

    iterator insert(const_iterator pos, T&& val)
    {
        return emplace(pos, std::move(val));
    }

    template <typename... Args>
    reference emplace_back(Args&&... args)
    {
        if(m_size == capacity())
            throw_length_error();
        pointer new_val = std::construct_at<T>(data() + m_size, std::forward<Args>(args)...);
        ++m_size;

        return *new_val;
    }

    void push_back(const value_type& val)
    {
        emplace_back(val);
    }

    void push_back(value_type&& val)
    {
        emplace_back(std::move(val));
    }

    void pop_back() noexcept
    {
        PAPILIO_ASSERT(!empty());

        std::destroy_at(data() + m_size - 1);
        --m_size;
    }

private:
    static_storage<sizeof(T) * Capacity, alignof(T)> m_buf;
    size_type m_size = 0;

    void* getbuf() noexcept
    {
        return m_buf.data();
    }

    const void* getbuf() const noexcept
    {
        return m_buf.data();
    }
};

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(pop)
#endif

namespace detail
{
    template <typename Compare>
    concept is_transparent_helper = requires() {
        typename Compare::is_transparent;
    };

    template <typename Compare, bool IsTransparent>
    class value_compare_base;

    template <typename Compare>
    class value_compare_base<Compare, false> : protected Compare
    {
    protected:
        value_compare_base() = default;
        value_compare_base(const value_compare_base&) = default;
        value_compare_base(value_compare_base&&) = default;

        value_compare_base(Compare comp)
            : Compare(comp) {}
    };

    template <typename Compare>
    class value_compare_base<Compare, true> : protected Compare
    {
    public:
        using is_transparent = void;

    protected:
        value_compare_base() = default;
        value_compare_base(const value_compare_base&) = default;
        value_compare_base(value_compare_base&&) = default;

        value_compare_base(Compare comp)
            : Compare(comp) {}
    };

    class fixed_flat_map_impl_base
    {
    public:
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        [[noreturn]]
        static void throw_out_of_range();
    };
} // namespace detail

template <typename Compare>
struct is_transparent :
    public std::bool_constant<detail::is_transparent_helper<Compare>>
{};

template <typename Compare>
inline constexpr bool is_transparent_v = is_transparent<Compare>::value;

template <typename Key, typename T, std::size_t Capacity, typename Compare = std::less<>>
class fixed_flat_map : public detail::fixed_flat_map_impl_base
{
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;
    using underlying_type = fixed_vector<value_type, Capacity>;
    using key_compare = Compare;
    using reference = value_type&;
    using const_reference = value_type&;
    using iterator = underlying_type::iterator;
    using const_iterator = underlying_type::const_iterator;

    class value_compare :
        public detail::value_compare_base<key_compare, is_transparent_v<Compare>>
    {
        using base = detail::value_compare_base<key_compare, is_transparent_v<Compare>>;

    public:
        value_compare() = default;
        value_compare(const value_compare&) = default;
        value_compare(value_compare&&) = default;

        bool operator()(const value_type& lhs, const value_type& rhs) const
        {
            return as_key_comp()(lhs.first, rhs.first);
        }

        template <typename T1, typename T2>
        bool operator()(T1&& lhs, T2&& rhs) const requires is_transparent_v<Compare>
        {
            return as_key_comp()(std::forward<T1>(lhs), std::forward<T2>(rhs));
        }

        key_compare key_comp() const
        {
            return as_key_comp();
        }

    private:
        friend class fixed_flat_map;

        using base::base;

        const key_compare& as_key_comp() const noexcept
        {
            return *this;
        }
    };

    fixed_flat_map()
        : m_data() {}

    // element access

    T& at(const Key& k)
    {
        auto it = find(k);
        if(it == end())
        {
            throw_out_of_range();
        }

        return it->second;
    }

    const T& at(const Key& k) const
    {
        auto it = find(k);
        if(it == end())
        {
            throw_out_of_range();
        }

        return it->second;
    }

    // iterators

    iterator begin() noexcept
    {
        return get_storage().begin();
    }

    iterator end() noexcept
    {
        return get_storage().end();
    }

    const_iterator begin() const noexcept
    {
        return get_storage().begin();
    }

    const_iterator end() const noexcept
    {
        return get_storage().end();
    }

    // capacity

    [[nodiscard]]
    bool empty() const noexcept
    {
        return get_storage().empty();
    }

    [[nodiscard]]
    size_type size() const noexcept
    {
        return get_storage().size();
    }

    [[nodiscard]]
    static size_type max_size() noexcept
    {
        return underlying_type::max_size();
    }

    template <typename U>
    std::pair<iterator, bool> insert_or_assign(const Key& k, U&& val)
    {
        static_assert(std::is_assignable_v<mapped_type&, U&&>);

        auto pos = lower_bound(k);
        if(pos != end() && is_equal(pos->first, k, get_comp().as_key_comp()))
        {
            pos->second = std::forward<U>(val);

            return std::make_pair(pos, false);
        }

        get_storage().emplace(
            pos,
            k,
            std::forward<U>(val)
        );

        return std::make_pair(pos, true);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const Key& k, Args&&... args)
    {
        auto pos = lower_bound(k);
        if(pos != end() && is_equal(pos->first, k, get_comp().as_key_comp()))
        {
            return std::make_pair(pos, false);
        }

        get_storage().emplace(
            pos,
            std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(std::forward<Args>(args))...
        );
        return std::make_pair(
            pos,
            true
        );
    }

    // lookup

    [[nodiscard]]
    iterator find(const Key& k)
    {
        auto it = lower_bound(k);
        return is_equal(it->first, k, get_comp().as_key_comp()) ? it : end();
    }

    [[nodiscard]]
    const_iterator find(const Key& k) const
    {
        auto it = lower_bound(k);
        return is_equal(it->first, k, get_comp().as_key_comp()) ? it : end();
    }

    [[nodiscard]]
    bool contains(const Key& k) const
    {
        return find(k) != end();
    }

    [[nodiscard]]
    iterator lower_bound(const Key& k)
    {
        return lower_bound_impl(begin(), end(), k, get_comp().as_key_comp());
    }

    [[nodiscard]]
    const_iterator lower_bound(const Key& k) const
    {
        return lower_bound_impl(begin(), end(), k, get_comp().as_key_comp());
    }

private:
    compressed_pair<
        underlying_type,
        value_compare>
        m_data;

    underlying_type& get_storage() noexcept
    {
        return m_data.first();
    }

    const underlying_type& get_storage() const noexcept
    {
        return m_data.first();
    }

    const value_compare& get_comp() const noexcept
    {
        return m_data.second();
    }

    template <typename T1, typename T2, typename Comp>
    static bool is_equal(T1&& lhs, T2&& rhs, Comp&& c)
    {
        return !c(lhs, rhs) && !c(rhs, lhs);
    }

    template <typename Iterator, typename K, typename Pred>
    static Iterator lower_bound_impl(Iterator start, Iterator stop, K&& k, Pred&& pred)
    {
        if constexpr(!is_transparent_v<std::remove_cvref_t<Pred>>)
        {
            static_assert(
                std::is_same_v<std::remove_cvref_t<K>, Key>,
                "transparent compare not available"
            );
        }

        std::iter_difference_t<Iterator> count = std::distance(start, stop);

        while(count > 0)
        {
            const std::iter_difference_t<Iterator> tmp_count = count / 2;
            const auto mid = std::next(start, tmp_count);
            if(pred(mid->first, std::forward<K>(k)))
            {
                start = std::next(mid);
                count -= tmp_count + 1;
            }
            else
            {
                count = tmp_count;
            }
        }

        return start;
    }
};
} // namespace papilio
