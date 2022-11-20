#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include "../macros.hpp"


namespace papilio::detail
{
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
            for(size_type i = 0; i < m_size; ++i)
            {
                std::allocator_traits<Allocator>::construct(
                    m_alloc,
                    m_data.getbuf() + i,
                    std::move(other.m_data.getbuf()[i])
                );
                assert(i < static_size());
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
            resize_mem(n);
        }
        void shrink_to_fit()
        {
            resize_mem(m_size);
        }

        // modifiers

        void clear() noexcept
        {
            destroy_all(data(), m_size);
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
                swap(m_size, other.m_size);
                swap(m_data.ptr, other.m_data.ptr);
                swap(m_capacity, other.m_capacity);
                swap(m_alloc, other.m_alloc);
            }
            else if(!m_dyn_alloc && other.m_dyn_alloc)
            {
                swap_data_static_to_dyn(other);
            }
            else if(!other.m_dyn_alloc && m_dyn_alloc)
            {
                other.swap_data_static_to_dyn(*this);
            }
            else
            {
                for(size_type i = 0; i < std::max(m_size, other.m_size); ++i)
                {
                    if(i < m_size && i >= other.m_size)
                    {
                        std::allocator_traits<Allocator>::construct(
                            m_alloc,
                            other.m_data.getbuf() + i,
                            std::move(m_data.getbuf()[i])
                        );
                        std::allocator_traits<Allocator>::destroy(
                            m_alloc,
                            m_data.getbuf() + i
                        );
                    }
                    else if(i >= m_size && i < other.m_size)
                    {
                        std::allocator_traits<Allocator>::construct(
                            m_alloc,
                            m_data.getbuf() + i,
                            std::move(other.m_data.getbuf()[i])
                        );
                        std::allocator_traits<Allocator>::destroy(
                            other.m_alloc,
                            other.m_data.getbuf() + i
                        );
                    }
                    else
                    {
                        swap(m_data.getbuf()[i], other.m_data.getbuf()[i]);
                    }
                }
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
            assert(m_size < m_capacity);
            std::allocator_traits<Allocator>::construct(
                m_alloc,
                data() + m_size,
                std::forward<Args>(args)...
            );
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
        void destroy_all(pointer ptr, size_type count)
        {
            for(size_type i = 0; i < count; ++i)
            {
                std::allocator_traits<Allocator>::destroy(
                    m_alloc,
                    ptr + i
                );
            }
        }

        void resize_mem(size_type new_cap)
        {
            if(new_cap > max_size())
                raise_length_error();

            assert(m_size <= new_cap);

            if(new_cap <= static_size())
            {
                if(!m_dyn_alloc)
                {
                    assert(m_capacity == static_size());
                    return;
                }

                pointer ptr = m_data.ptr;

                for(size_type i = 0; i < m_size; ++i)
                {
                    std::allocator_traits<Allocator>::construct(
                        m_alloc,
                        m_data.getbuf() + i,
                        std::move(ptr[i])
                    );
                }
                destroy_all(ptr, m_size);

                std::allocator_traits<Allocator>::deallocate(
                    m_alloc,
                    ptr,
                    m_capacity
                );

                m_dyn_alloc = false;
                m_capacity = static_size();

                return;
            }

            pointer new_mem = m_alloc.allocate(new_cap);
            try
            {
                pointer ptr = data();
                for(size_type i = 0; i < m_size; ++i)
                {
                    std::allocator_traits<Allocator>::construct(
                        m_alloc,
                        new_mem + i,
                        std::move(ptr[i])
                    );
                }
                destroy_all(ptr, m_size);
                free_mem();
                m_dyn_alloc = true;
                m_data.ptr = new_mem;
                m_capacity = new_cap;
            }
            catch(...)
            {
                std::allocator_traits<Allocator>::deallocate(
                    m_alloc,
                    new_mem,
                    new_cap
                );
                throw;
            }
        }

        // this function doesn't do anything on the allocator
        void swap_data_static_to_dyn(small_vector& other)
        {
            assert(!m_dyn_alloc);
            assert(other.m_dyn_alloc);

            pointer ptr = other.m_data.ptr;
            size_type tmp_size = m_size;

            other.m_dyn_alloc = false;
            for(size_type i = 0; i < tmp_size; ++i)
            {
                std::allocator_traits<Allocator>::construct(
                    m_alloc,
                    other.m_data.getbuf() + i,
                    std::move(m_data.getbuf()[i])
                );
            }
            clear();

            m_data.ptr = ptr;
            m_dyn_alloc = true;
            m_capacity = std::exchange(other.m_capacity, static_size());
            m_size = std::exchange(other.m_size, tmp_size);
        }
    };
}
