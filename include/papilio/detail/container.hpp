#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <array>
#include <utility>
#include <numeric>
#include <stdexcept>
#include "../macros.hpp"


namespace papilio::detail
{
    template <std::size_t Capacity>
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
        std::byte m_data[Capacity]{};
    };
    template <>
    class static_storage<0>
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

    class small_vector_base
    {
    public:
        using size_type = std::size_t;

        [[noreturn]]
        static void raise_out_of_range();
        [[noreturn]]
        static void raise_length_error();

    protected:
        [[nodiscard]]
        static size_type calc_mem_size(size_type current, size_type required) noexcept;
    };

    template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
    class small_vector : public small_vector_base
    {
        using base = detail::small_vector_base;
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = base::size_type;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = std::allocator_traits<Allocator>::pointer;
        using const_pointer = std::allocator_traits<Allocator>::const_pointer;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        small_vector() noexcept(noexcept(Allocator())) = default;
        explicit small_vector(const Allocator& alloc) noexcept
            : m_alloc(alloc) {}
        small_vector(const small_vector& other)
        {
            reserve(other.size());
            for(size_type i = 0; i < other.size(); ++i)
            {
                emplace_back_raw(other[i]);
            }
        }
        small_vector(small_vector&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
            : m_alloc(std::move(other.m_alloc))
        {
            if(other.m_dyn_alloc)
            {
                m_data.ptr = std::exchange(other.m_data.ptr, nullptr);
                m_dyn_alloc = std::exchange(other.m_dyn_alloc, false);
                m_capacity = std::exchange(other.m_capacity, static_size());
                m_size = std::exchange(other.m_size, 0);

                return;
            }

            m_size = other.m_size;
            PAPILIO_ASSUME(m_size <= static_size());
            PAPILIO_ASSUME(!m_dyn_alloc);
            for(size_type i = 0; i < m_size; ++i)
            {
                std::construct_at(
                    m_data.getbuf() + i,
                    std::move(other.m_data.getbuf()[i])
                );
            }

            other.clear();
        }
        template <typename Iterator>
        small_vector(Iterator first, Iterator last, const Allocator& alloc = Allocator())
            : m_alloc(alloc)
        {
            assign(first, last);
        }
        small_vector(std::initializer_list<value_type> il, const Allocator& alloc = Allocator())
            : m_alloc(alloc)
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
        small_vector& operator=(small_vector&& rhs)
            noexcept(std::is_nothrow_move_constructible_v<small_vector>)
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
                std::random_access_iterator_tag
            >;
            if constexpr(random_access)
            {
                reserve(std::distance(first, last));
                for(auto it = first; it != last; ++it)
                {
                    emplace_back_raw(*it);
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
            return m_alloc;
        }

        // element access

        reference at(size_type i)
        {
            if(i < size())
                return data()[i];
            raise_out_of_range();
        }
        const_reference at(size_type i) const
        {
            if(i < size())
                return data()[i];
            raise_out_of_range();
        }
        reference operator[](size_type i) noexcept
        {
            return data()[i];
        }
        const_reference operator[](size_type i) const noexcept
        {
            return data()[i];
        }

        reference front() noexcept
        {
            assert(!empty());
            return *begin();
        }
        const_reference front() const noexcept
        {
            assert(!empty());
            return *begin();
        }
        reference back() noexcept
        {
            assert(!empty());
            return *(end() - 1);
        }
        const_reference back() const noexcept
        {
            assert(!empty());
            return *(end() - 1);
        }

        [[nodiscard]]
        pointer data() noexcept
        {
            [[unlikely]]
            if(m_dyn_alloc)
                return m_data.ptr;
            else
                return m_data.getbuf();
        }
        [[nodiscard]]
        const_pointer data() const noexcept
        {
            [[unlikely]]
            if(m_dyn_alloc)
                return m_data.ptr;
            else
                return m_data.getbuf();
        }

        // iterators

        iterator begin() noexcept
        {
            return data();
        }
        iterator end() noexcept
        {
            return data() + m_size;
        }
        const_iterator begin() const noexcept
        {
            return data();
        }
        const_iterator end() const noexcept
        {
            return data() + m_size;
        }
        const_iterator cbegin() const noexcept
        {
            return begin();
        }
        const_iterator cend() const noexcept
        {
            return data() + m_size;
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
            return m_size == 0;
        }
        [[nodiscard]]
        size_type size() const noexcept
        {
            return m_size;
        }
        [[nodiscard]]
        size_type max_size() const noexcept
        {
            return std::numeric_limits<difference_type>::max();
        }
        [[nodiscard]]
        static constexpr size_type static_size() noexcept
        {
            return N;
        }
        [[nodiscard]]
        size_type capacity() const noexcept
        {
            return m_capacity;
        }
        [[nodiscard]]
        bool dynamic_allocated() const noexcept
        {
            return m_dyn_alloc;
        }
        void reserve(size_type n)
        {
            if(n <= m_capacity)
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
            m_size = 0;
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
            size_type new_size = m_size + 1;
            if(new_size > static_size())
                reserve(calc_mem_size(m_size, new_size));
            emplace_back_raw(std::forward<Args>(args)...);
        }

        void pop_back() noexcept
        {
            assert(!empty());
            std::allocator_traits<Allocator>::destroy(
                m_alloc,
                data() + m_size - 1
            );
            --m_size;
        }

        void resize(size_type count, const value_type& val)
        {
            if(count < m_size)
            {
                while(m_size != count)
                    pop_back();
            }
            else if(count > m_size)
            {
                reserve(count);
                while(m_size != count)
                    emplace_back_raw(val);
            }
        }
        void resize(size_type count) requires std::is_default_constructible_v<value_type>
        {
            resize(count, value_type());
        }

        void swap(small_vector& other) noexcept
        {
            using std::swap;

            if(m_dyn_alloc && other.m_dyn_alloc)
            {
                // both are dynamic allocated
                swap(m_size, other.m_size);
                swap(m_data.ptr, other.m_data.ptr);
                swap(m_capacity, other.m_capacity);
                if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value)
                    swap(m_alloc, other.m_alloc);
            }
            else if(!m_dyn_alloc && other.m_dyn_alloc)
            {
                swap_static_to_dyn(other);
            }
            else if(!other.m_dyn_alloc && m_dyn_alloc)
            {
                other.swap_static_to_dyn(*this);
            }
            else
            {
                // both are using static storage
                PAPILIO_ASSUME(!m_dyn_alloc && !other.m_dyn_alloc);
                for(size_type i = 0; i < std::max(m_size, other.m_size); ++i)
                {
                    if(i < m_size && i >= other.m_size)
                    {
                        // this->size() > other.size()
                        // copy elements to the other one
                        std::construct_at(
                            other.m_data.getbuf() + i,
                            std::move(m_data.getbuf()[i])
                        );
                        std::destroy_at(m_data.getbuf() + i);
                    }
                    else if(i >= m_size && i < other.m_size)
                    {
                        // this->size() < other.size()
                        // copy elements from the other one
                        std::construct_at(
                            m_data.getbuf() + i,
                            std::move(other.m_data.getbuf()[i])
                        );
                        std::destroy_at(other.m_data.getbuf() + i);
                    }
                    else
                    {
                        // same size
                        // 
                        swap(m_data.getbuf()[i], other.m_data.getbuf()[i]);
                    }
                }

                swap(m_size, other.m_size);
                if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value)
                    swap(m_alloc, other.m_alloc);
            }
        }

    private:
        union underlying_type
        {
            underlying_type() noexcept
                : buf{} {}

            char buf[sizeof(value_type) * N];
            pointer ptr;

            pointer getbuf() noexcept
            {
                return reinterpret_cast<pointer>(buf);
            }
            const_pointer getbuf() const noexcept
            {
                return reinterpret_cast<const_pointer>(buf);
            }
        };

        size_type m_size = 0;
        size_type m_capacity = static_size();
        underlying_type m_data;
        bool m_dyn_alloc = false;
        PAPILIO_NO_UNIQUE_ADDRESS allocator_type m_alloc;

        template <typename... Args>
        void emplace_back_raw(Args&&... args)
        {
            PAPILIO_ASSUME(m_size < m_capacity);
            if(!m_dyn_alloc) [[likely]]
            {
                std::construct_at(
                    data() + m_size,
                    std::forward<Args>(args)...
                );
            }
            else
            {
                std::allocator_traits<Allocator>::construct(
                    m_alloc,
                    data() + m_size,
                    std::forward<Args>(args)...
                );
            }
            ++m_size;
        }

        void free_mem() noexcept
        {
            if(!m_dyn_alloc)
                return;
            std::allocator_traits<Allocator>::deallocate(
                m_alloc,
                m_data.ptr,
                m_capacity
            );
        }
        // Note: This function doesn't automatically set the m_size to 0.
        void destroy_all() noexcept
        {
            if(!m_dyn_alloc)
            {
                std::destroy_n(data(), m_size);
            }
            else
            {
                for(size_type i = 0; i < m_size; ++i)
                {
                    std::allocator_traits<Allocator>::destroy(
                        m_alloc,
                        m_data.ptr + i
                    );
                }
            }
        }

#ifdef PAPILIO_COMPILER_MSVC
#   pragma warning(push)
#   pragma warning(disable:26800) // lifetime.1
#endif

        void shrink_mem()
        {
            if(m_size <= static_size())
            {
                if(!m_dyn_alloc)
                {
                    assert(m_capacity == static_size());
                    return;
                }

                // move from dynamic allocated memory to static storage
                pointer tmp_ptr = m_data.ptr;
                m_dyn_alloc = false;

                for(size_type i = 0; i < m_size; ++i)
                {
                    std::construct_at(
                        m_data.getbuf() + i,
                        std::move(*(tmp_ptr + i))
                    );
                    std::allocator_traits<Allocator>::destroy(
                        m_alloc,
                        tmp_ptr + i
                    );
                }

                std::allocator_traits<Allocator>::deallocate(
                    m_alloc,
                    tmp_ptr,
                    m_capacity
                );

                m_capacity = static_size();
            }
        }

#ifdef PAPILIO_COMPILER_MSVC
#   pragma warning(pop)
#endif

        void grow_mem(size_type new_cap)
        {
            assert(m_capacity < new_cap);
            if(new_cap > max_size())
                raise_length_error();

            pointer new_mem = m_alloc.allocate(new_cap);
            size_type i = 0;
            try
            {
                pointer tmp_ptr = data();
                for(i = 0; i < m_size; ++i)
                {
                    std::allocator_traits<Allocator>::construct(
                        m_alloc,
                        new_mem + i,
                        std::move(*(tmp_ptr + i))
                    );
                }
                destroy_all(); // m_size is unchanged
                free_mem();
                m_dyn_alloc = true;
                m_data.ptr = new_mem;
                m_capacity = new_cap;
            }
            catch(...)
            {
                for(size_type j = 0; j < i; ++j)
                {
                    std::allocator_traits<Allocator>::destroy(
                        m_alloc,
                        new_mem + j
                    );
                }
                std::allocator_traits<Allocator>::deallocate(
                    m_alloc,
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

            PAPILIO_ASSUME(!m_dyn_alloc);

            pointer tmp_ptr = other.m_data.ptr;
            size_type tmp_size = m_size;
            other.m_dyn_alloc = false;

            for(size_type i = 0; i < tmp_size; ++i)
            {
                std::construct_at(
                    other.m_data.getbuf() + i,
                    std::move(*(m_data.getbuf() + i))
                );
            }
            destroy_all();

            m_data.ptr = tmp_ptr;
            m_dyn_alloc = true;
            m_capacity = std::exchange(other.m_capacity, static_size());
            m_size = std::exchange(other.m_size, tmp_size);
            if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value)
                swap(m_alloc, other.m_alloc);
        }
    };

    class fixed_vector_base
    {
    public:
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        [[noreturn]]
        static void raise_out_of_range();
        [[noreturn]]
        static void raise_length_error();
    };

    template <typename T, std::size_t Capacity>
    class fixed_vector : public fixed_vector_base
    {
    public:
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
            [[unlikely]]
            if(pos >= size())
                raise_out_of_range();
            return data()[pos];
        }
        const_reference at(size_type pos) const
        {
            [[unlikely]]
            if(pos >= size())
                raise_out_of_range();
            return data()[pos];
        }

        reference operator[](size_type pos) noexcept
        {
            return data()[pos];
        }
        const_reference operator[](size_type pos) const noexcept
        {
            return data()[pos];
        }

        reference front() noexcept
        {
            return *begin();
        }
        const_reference front() const noexcept
        {
            return *begin();
        }
        reference back() noexcept
        {
            return *(end() - 1);
        }
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

        iterator begin() noexcept { return data(); }
        iterator end() noexcept { return data() + size(); }
        const_iterator begin() const noexcept { return data(); }
        const_iterator end() const noexcept { return data() + size(); }
        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        // capacity

        bool empty() const noexcept
        {
            return m_size == 0;
        }
        size_type size() const noexcept
        {
            return m_size;
        }
        static constexpr size_type max_size() noexcept
        {
            return Capacity;
        }
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
            assert(pos >= cbegin());
            assert(pos <= cend());

            if(pos == cend())
            {
                return std::addressof(emplace_back(std::forward<Args>(args)...));
            }
            else if(m_size == capacity())
            {
                raise_length_error();
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
                raise_length_error();
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
            assert(!empty());

            std::destroy_at(data() + m_size - 1);
            --m_size;
        }

    private:
        static_storage<sizeof(T)* Capacity> m_buf;
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

    template <bool IsTransparent>
    class value_compare_base {};
    template <>
    class value_compare_base<true>
    {
        using is_transparent = void;
    };

    template <typename Compare>
    inline constexpr bool is_transparent_v = requires()
    {
        typename Compare::is_transparent;
    };

    class fixed_flat_map_base
    {
    public:
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        [[noreturn]]
        static void raise_out_of_range();
    };

    template <typename Key, typename T, std::size_t Capacity, typename Compare = std::less<>>
    class fixed_flat_map : public fixed_flat_map_base
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

        class value_compare : value_compare_base<is_transparent_v<Compare>>
        {
        public:
            value_compare() = default;
            value_compare(const value_compare&) = default;
            value_compare(value_compare&&) = default;

            bool operator()(const value_type& lhs, const value_type& rhs) const
            {
                return m_comp(lhs.first, rhs.first);
            }
            template <typename T1, typename T2>
            bool operator()(T1&& lhs, T2&& rhs) const requires is_transparent_v<Compare>
            {
                return m_comp(std::forward<T1>(lhs), std::forward<T2>(rhs));
            }

            Compare key_comp() const
            {
                return m_comp;
            }

        protected:
            friend class fixed_flat_map;

            value_compare(Compare comp_)
                : comp(comp_) {}

            key_compare comp;
        };

        fixed_flat_map()
            : m_storage(), m_comp() {}

        // element access

        T& at(const Key& k)
        {
            auto it = find(k);
            if(it == end())
            {
                raise_out_of_range();
            }

            return it->second;
        }
        const T& at(const Key& k) const
        {
            auto it = find(k);
            if(it == end())
            {
                raise_out_of_range();
            }

            return it->second;
        }

        // iterators

        iterator begin() noexcept { return m_storage.begin(); }
        iterator end() noexcept { return m_storage.end(); }
        const_iterator begin() const noexcept { return m_storage.begin(); }
        const_iterator end() const noexcept { return m_storage.end(); }

        // capacity

        bool empty() const noexcept
        {
            return m_storage.empty();
        }
        size_type size() const noexcept
        {
            return m_storage.size();
        }
        static size_type max_size() noexcept
        {
            return underlying_type::max_size();
        }

        template <typename U>
        std::pair<iterator, bool> insert_or_assign(const Key& k, U&& val)
        {
            static_assert(std::is_assignable_v<mapped_type&, U&&>);

            auto pos = lower_bound(k);
            if(pos != end() && is_equal(pos->first, k, m_comp.comp))
            {
                pos->second = std::forward<U>(val);

                return std::make_pair(pos, false);
            }

            m_storage.emplace(
                pos,
                k, std::forward<U>(val)
            );

            return std::make_pair(pos, true);
        }

        template <typename... Args>
        std::pair<iterator, bool> try_emplace(const Key& k, Args&&... args)
        {
            auto pos = lower_bound(k);
            if(pos != end() && is_equal(pos->first, k, m_comp.comp))
            {
                return std::make_pair(pos, false);
            }

            m_storage.emplace(
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

        iterator find(const Key& k)
        {
            auto it = lower_bound(k);
            return is_equal(it->first, k, m_comp.comp) ? it : end();
        }
        const_iterator find(const Key& k) const
        {
            auto it = lower_bound(k);
            return is_equal(it->first, k, m_comp.comp) ? it : end();
        }
        bool contains(const Key& k) const
        {
            return find(k) != end();
        }
        iterator lower_bound(const Key& k)
        {
            return lower_bound_impl(begin(), end(), k, m_comp.comp);
        }
        const_iterator lower_bound(const Key& k) const
        {
            return lower_bound_impl(begin(), end(), k, m_comp.comp);
        }

    private:
        underlying_type m_storage;
        PAPILIO_NO_UNIQUE_ADDRESS value_compare m_comp;

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
}
