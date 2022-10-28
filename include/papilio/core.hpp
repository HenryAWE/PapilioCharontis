#pragma once

#include <cassert>
#include <string>
#include <variant>
#include <concepts>
#include <stdexcept>
#include <vector>
#include <map>
#include <charconv>
#include <limits>
#include <iterator>
#include <algorithm>
#include <typeinfo>
#include "macros.hpp"
#include "utf8.hpp"
#include "error.hpp"


namespace papilio
{
    // forward declarations
    class slice;
    class indexing_value;
    class attribute_name;
    class format_arg;
    template <typename OutputIt, typename Store>
    class basic_format_context;
    class dynamic_format_context;
    class format_spec_parse_context;

    template <typename T, typename CharT>
    concept basic_string_like =
        std::is_same_v<std::decay_t<T>, CharT*> ||
        std::is_same_v<std::decay_t<T>, const CharT*> ||
        std::is_same_v<T, std::basic_string<CharT>> ||
        std::is_same_v<T, std::basic_string_view<CharT>>;
    template <typename T>
    concept string_like = basic_string_like<T, char>;

    namespace detail
    {
        [[nodiscard]]
        constexpr bool is_digit(char ch) noexcept
        {
            return '0' <= ch && ch <= '9';
        }

        template <typename CharT>
        [[nodiscard]]
        constexpr bool is_identifier_ch(CharT ch, bool first = false) noexcept
        {
            bool digit = is_digit(ch);
            if(first && digit)
                return false;

            return ('A' <= ch && ch <= 'z') || digit || ch == '_';
        }

        template <typename T>
        concept char_type =
            std::is_same_v<T, char> ||
            std::is_same_v<T, wchar_t> ||
            std::is_same_v<T, char16_t> ||
            std::is_same_v<T, char32_t> ||
            std::is_same_v<T, char8_t>;
    }

    namespace detail
    {
        class small_vector_base
        {
        public:
            using size_type = std::size_t;

            [[noreturn]]
            static void raise_out_of_range()
            {
                throw std::out_of_range("small vector index out of range");
            }
            [[noreturn]]
            static void raise_length_error()
            {
                throw std::length_error("length error");
            }

        protected:
            [[nodiscard]]
            static size_type calc_mem_size(size_type current, size_type required) noexcept
            {
                assert(current < required);
                if(current <= 1)
                    current = 2;
                while(current < required)
                    current += current / 2;
                return current;
            }
        };
    }

    template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
    class small_vector : public detail::small_vector_base
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
        const_pointer data () const noexcept
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

        PAPILIO_NO_UNIQUE_ADDRESS
        allocator_type m_alloc;

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

    namespace script
    {
        class variable
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;
            using string_view_type = std::basic_string_view<char_type>;
            using int_type = std::int64_t;
            using float_type = long double;

            using underlying_type = std::variant<
                bool,
                int_type,
                float_type,
                string_container
            >;

            variable() = delete;
            variable(const variable&) = default;
            variable(variable&&) noexcept = default;
            variable(bool v)
                : m_var(v) {}
            template <std::integral T>
            variable(T i)
                : m_var(static_cast<int_type>(i)) {}
            template <std::floating_point T>
            variable(T f)
                : m_var(static_cast<float_type>(f)) {}
            variable(string_container str)
                : m_var(std::move(str)) {}
            template <string_like String>
            variable(String&& str)
                : m_var(std::in_place_type<string_container>, std::forward<String>(str)) {}
            template <string_like String>
            variable(independent_t, String&& str)
                : m_var(std::in_place_type<string_container>, independent, std::forward<String>(str)) {}

            variable& operator=(const variable&) = default;
            variable& operator=(bool v)
            {
                m_var = v;
                return *this;
            }
            template <std::integral T>
            variable& operator=(T i)
            {
                m_var = static_cast<int_type>(i);
                return *this;
            }
            template <std::floating_point T>
            variable& operator=(T f)
            {
                m_var = static_cast<float_type>(f);
                return *this;
            }
            variable& operator=(string_type str)
            {
                m_var = std::move(str);
                return *this;
            }
            variable& operator=(string_container str)
            {
                m_var.emplace<string_container>(std::move(str));
                return *this;
            }
            template <string_like String>
            variable& operator=(String&& str)
            {
                m_var.emplace<string_container>(std::forward<String>(str));
                return *this;
            }

            template <typename T>
            [[nodiscard]]
            bool holds() const noexcept
            {
                return std::holds_alternative<T>(m_var);
            }

            template <typename T>
            [[nodiscard]]
            const T& get() const noexcept
            {
                assert(holds<T>());
                return *std::get_if<T>(&m_var);
            }

            template <typename T>
            [[nodiscard]]
            T as() const
            {
                auto visitor = [](auto&& v)->T
                {
                    using std::is_same_v;
                    using U = std::remove_cvref_t<decltype(v)>;

                    if constexpr(is_same_v<T, bool>)
                    {
                        if constexpr(is_same_v<U, string_container>)
                        {
                            return !v.empty();
                        }
                        else
                        {
                            return static_cast<bool>(v);
                        }
                    }
                    else if constexpr(std::integral<T> ||std::floating_point<T>)
                    {
                        if constexpr(is_same_v<U, string_container>)
                        {
                            invalid_conversion();
                        }
                        else // ind_type and float_type
                        {
                            return static_cast<T>(v);
                        }
                    }
                    else if constexpr(is_same_v<T, string_container> || string_like<T>)
                    {
                        if constexpr(is_same_v<U, string_container>)
                        {
                            return T(string_view_type(v));
                        }
                        else
                        {
                            invalid_conversion();
                        }
                    }
                    else
                    {
                        static_assert(!sizeof(T), "invalid type");
                    }
                };

                return std::visit(visitor, m_var);
            }

            underlying_type& to_underlying() noexcept
            {
                return m_var;
            }
            const underlying_type& to_underlying() const noexcept
            {
                return m_var;
            }

            [[nodiscard]]
            std::partial_ordering compare(const variable& var) const;
            [[nodiscard]]
            std::partial_ordering operator<=>(const variable& rhs) const
            {
                return compare(rhs);
            }

            [[nodiscard]]
            bool equal(
                const variable& var,
                float_type epsilon = std::numeric_limits<float_type>::epsilon()
            ) const noexcept;
            [[nodiscard]]
            bool operator==(const variable& rhs) const noexcept
            {
                return equal(rhs);
            }

            [[noreturn]]
            static void invalid_conversion()
            {
                throw std::runtime_error("invalid conversion");
            }

        private:
            underlying_type m_var;
        };

        template <typename T>
        struct is_variable_type
        {
            static constexpr bool value =
                std::is_same_v<T, bool> ||
                std::is_same_v<T, variable::int_type> ||
                std::is_same_v<T, variable::float_type> ||
                std::is_same_v<T, string_container>;
        };
        template <typename T>
        inline constexpr bool is_variable_type_v = is_variable_type<T>::value;
    }

    enum class format_align : std::uint8_t
    {
        default_align = 0,
        left,
        middle,
        right
    };
    enum class format_sign
    {
        default_sign = 0,
        positive,
        negative,
        space
    };

    class format_spec_parse_context;

    namespace detail
    {
        struct std_format_spec
        {
            using char_type = char;

            char_type fill = ' ';
            format_align align = format_align::default_align;
            format_sign sign = format_sign::default_sign;
            bool alternate_form = false; // specified by '#'
            bool fill_zero = false;
            std::size_t width = 0;
            bool use_locale = false;
            char_type type_char = '\0';
        };

        std_format_spec parse_std_format_spec(format_spec_parse_context& ctx);
    }

    class common_format_spec
    {
    public:
        common_format_spec() = default;
        common_format_spec(const common_format_spec&) noexcept = default;
        common_format_spec(format_spec_parse_context& spec_ctx)
        {
            parse(spec_ctx);
        }

        void parse(format_spec_parse_context& spec_ctx);
        void reset() noexcept
        {
            *this = common_format_spec();
        }

        [[nodiscard]]
        static constexpr bool is_align_spec(char32_t ch) noexcept
        {
            return
                ch == '<' ||
                ch == '^' ||
                ch == '>';
        }
        [[nodiscard]]
        static constexpr format_align get_align(char32_t ch) noexcept
        {
            switch(ch)
            {
                using enum format_align;
            case '<':
                return left;
            case '^':
                return middle;
            case '>':
                return right;
            default:
                return default_align;
            }
        }

        [[nodiscard]]
        static constexpr bool is_sign_spec(char32_t ch) noexcept
        {
            return
                ch == '+' ||
                ch == ' ' ||
                ch == '-';
        }
        [[nodiscard]]
        static constexpr format_sign get_sign(char32_t ch) noexcept
        {
            switch(ch)
            {
                using enum format_sign;
            [[likely]] case '+':
                return positive;
            case ' ':
                return space;
            default:
            case '-':
                return negative;
            }
        }

        [[nodiscard]]
        bool has_fill() const noexcept
        {
            return m_fill != U'\0';
        }
        [[nodiscard]]
        utf8::codepoint fill() const noexcept
        {
            return m_fill;
        }
        void fill(utf8::codepoint cp) noexcept
        {
            m_fill = cp;
        }
        [[nodiscard]]
        utf8::codepoint fill_or(utf8::codepoint or_cp) const noexcept
        {
            return has_fill() ? m_fill : or_cp;
        }
        [[nodiscard]]
        format_align align() const noexcept
        {
            return m_align;
        }
        void align(format_align val) noexcept
        {
            m_align = val;
        }
        [[nodiscard]]
        format_sign sign() const noexcept
        {
            return m_sign;
        }
        void sign(format_sign val) noexcept
        {
            m_sign = val;
        }
        [[nodiscard]]
        bool alternate_form() const noexcept
        {
            return m_alternate_form;
        }
        void alternate_form(bool val) noexcept
        {
            m_alternate_form = val;
        }
        [[nodiscard]]
        bool fill_zero() const noexcept
        {
            return m_fill_zero;
        }
        void fill_zero(bool val) noexcept
        {
            m_fill_zero = val;
        }
        [[nodiscard]]
        std::size_t width() const noexcept
        {
            return m_width;
        }
        void width(std::size_t val) noexcept
        {
            m_width = val;
        }
        [[nodiscard]]
        std::size_t precision() const noexcept
        {
            return m_precision;
        }
        void precision(std::size_t val) noexcept
        {
            m_precision = val;
        }
        [[nodiscard]]
        bool use_locale() const noexcept
        {
            return m_use_locale;
        }
        void use_locale(bool val) noexcept
        {
            m_use_locale = val;
        }
        [[nodiscard]]
        bool has_type_char() const noexcept
        {
            return m_type_char != U'\0';
        }
        [[nodiscard]]
        utf8::codepoint type_char() const noexcept
        {
            return m_type_char;
        }
        void type_char(utf8::codepoint cp) noexcept
        {
            m_type_char = cp;
        }
        [[nodiscard]]
        utf8::codepoint type_char_or(utf8::codepoint or_cp) const noexcept
        {
            return has_type_char() ? m_type_char : or_cp;
        }

    private:
        utf8::codepoint m_fill = U'\0';
        format_align m_align = format_align::default_align;
        format_sign m_sign = format_sign::default_sign;
        bool m_alternate_form = false; // specified by '#'
        bool m_fill_zero = false;
        std::size_t m_width = 0;
        std::size_t m_precision = -1;
        bool m_use_locale = false;
        utf8::codepoint m_type_char = U'\0';
    };

    template <typename T>
    struct named_arg
    {
        using named_arg_tag = void;

        const char* name;
        const T& value;

        named_arg() = delete;
        constexpr named_arg(const named_arg&) noexcept = default;
        constexpr named_arg(const char* name_, const T& value_) noexcept
            : name(name_), value(value_) {}

        named_arg& operator=(const named_arg&) = delete;
    };

    template <typename T>
    constexpr named_arg<T> arg(const char* name, const T& value) noexcept
    {
        return named_arg<T>(name, value);
    }

    inline namespace literals
    {
        struct named_arg_proxy
        {
            const char* name;

            named_arg_proxy() = delete;
            named_arg_proxy(const named_arg_proxy&) = delete;
            constexpr named_arg_proxy(const char* name_) noexcept
                : name(name_) {}

            template <typename T>
            constexpr named_arg<T> operator=(const T& value) noexcept
            {
                return named_arg(name, value);
            }
        };

        [[nodiscard]]
        constexpr named_arg_proxy operator""_a(const char* name, std::size_t) noexcept
        {
            return named_arg_proxy(name);
        }
    }

    class slice
    {
    public:
        using index_type = std::make_signed_t<std::size_t>; // ssize_t

        static constexpr index_type npos = std::numeric_limits<index_type>::max();

        constexpr slice() noexcept
            : m_begin(0), m_end(npos) {}
        constexpr slice(const slice&) noexcept = default;
        constexpr explicit slice(index_type begin_, index_type end_) noexcept
            : m_begin(begin_), m_end(end_) {}

        slice& operator=(const slice&) noexcept = default;

        constexpr void normalize(index_type length) noexcept
        {
            if(m_begin < 0)
                m_begin = length + m_begin;
            if(m_end)
                m_end = length + m_end;
        }

        [[nodiscard]]
        constexpr index_type begin() const noexcept
        {
            return m_begin;
        }
        [[nodiscard]]
        constexpr index_type end() const noexcept
        {
            return m_end;
        }

    private:
        index_type m_begin;
        index_type m_end;
    };

    class indexing_value
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using index_type = std::make_signed_t<std::size_t>; // ssize_t
        using underlying_type = std::variant<
            index_type,
            slice,
            string_container
        >;

        indexing_value() = delete;
        indexing_value(const indexing_value&) = default;
        indexing_value(indexing_value&&) = default;
        indexing_value(index_type index)
            : m_val(index) {}
        indexing_value(slice s)
            : m_val(s) {}
        indexing_value(string_container key)
            : m_val(std::move(key)) {}
        template <string_like String>
        indexing_value(String&& key)
            : m_val(std::in_place_type<string_container>, std::forward<String>(key)) {}
        template <string_like String>
        indexing_value(independent_t, String&& key)
            : m_val(std::in_place_type<string_container>, independent, std::forward<String>(key)) {}

        template <typename T>
        [[nodiscard]]
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_val);
        }

        [[nodiscard]]
        bool is_index() const noexcept
        {
            return holds<index_type>();
        }
        [[nodiscard]]
        bool is_slice() const noexcept
        {
            return holds<slice>();
        }
        [[nodiscard]]
        bool is_key() const noexcept
        {
            return holds<string_container>();
        }

        [[nodiscard]]
        index_type as_index() const noexcept
        {
            // use std::get_if to avoid exception
            return *std::get_if<index_type>(&m_val);
        }
        [[nodiscard]]
        const slice& as_slice() const noexcept
        {
            return *std::get_if<slice>(&m_val);
        }
        [[nodiscard]]
        const string_container& as_key() const noexcept
        {
            return *std::get_if<string_container>(&m_val);
        }

        [[nodiscard]]
        underlying_type& to_underlying() noexcept
        {
            return m_val;
        }
        [[nodiscard]]
        const underlying_type& to_underlying() const noexcept
        {
            return m_val;
        }

    private:
        underlying_type m_val;
    };
    class attribute_name
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        attribute_name() = delete;
        attribute_name(const attribute_name&) = default;
        attribute_name(attribute_name&&) noexcept = default;
        template <typename... Args> requires(std::constructible_from<string_container, Args...>)
        attribute_name(Args&&... args)
            : m_name(std::forward<Args>(args)...) {}

        [[nodiscard]]
        bool operator==(const attribute_name& rhs) const noexcept = default;
        [[nodiscard]]
        friend bool operator==(const attribute_name& lhs, const string_type& rhs) noexcept
        {
            return lhs.m_name == rhs;
        }
        [[nodiscard]]
        friend bool operator==(const string_type& lhs, const attribute_name& rhs) noexcept
        {
            return lhs == rhs.m_name;
        }
        [[nodiscard]]
        friend bool operator==(const attribute_name& lhs, string_view_type rhs) noexcept
        {
            return lhs.m_name == rhs;
        }
        [[nodiscard]]
        friend bool operator==(string_view_type lhs, const attribute_name& rhs) noexcept
        {
            return lhs == rhs.m_name;
        }
        [[nodiscard]]
        friend bool operator==(const attribute_name& lhs, const char* rhs) noexcept
        {
            return lhs.m_name == rhs;
        }
        [[nodiscard]]
        friend bool operator==(const char* lhs, const attribute_name& rhs) noexcept
        {
            return lhs == rhs.m_name;
        }

        [[nodiscard]]
        const string_container& name() const noexcept
        {
            return m_name;
        }
        [[nodiscard]]
        operator string_view_type() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        static bool validate(string_view_type name) noexcept;

    private:
        string_container m_name;
    };
    class invalid_attribute : public std::invalid_argument
    {
    public:
        invalid_attribute(attribute_name attr)
            : invalid_argument("invalid attribute name \"" + std::string(attr.name()) + '\"'),
            m_attr(independent, attr.name()) {}

        [[nodiscard]]
        const attribute_name& attr() const noexcept
        {
            return m_attr;
        }

    private:
        attribute_name m_attr;
    };

    template <typename T>
    class formatter;

    template <typename T>
    class formatter_traits
    {
    public:
        using type = T;
        using formatter_type = formatter<T>;

        template <typename Context = dynamic_format_context>
        [[nodiscard]]
        static constexpr bool has_formatter() noexcept;

        template <typename Context>
        static void format(format_spec_parse_context& spec, const type& val, Context& ctx);
    };

    template <typename T>
    struct accessor {};

    template <typename T>
    class accessor_traits
    {
    public:
        using type = T;
        using accessor_type = accessor<type>;

        [[nodiscard]]
        static constexpr bool has_index() noexcept
        {
            return requires() { typename accessor_type::has_index; };
        }
        [[nodiscard]]
        static constexpr bool has_custom_index() noexcept
        {
            if constexpr(has_index())
            {
                return requires(T object, indexing_value::index_type i) { accessor_type::get(object, i); };
            }
            else
                return false;
        }

        [[nodiscard]]
        static constexpr bool has_key() noexcept
        {
            return requires() { typename accessor_type::has_key; };
        }
        [[nodiscard]]
        static constexpr bool has_custom_key() noexcept
        {
            if constexpr(has_key())
            {
                return requires(T object, indexing_value::string_view_type str) { accessor_type::get(object, str); };
            }
            else
                return false;
        }

        [[nodiscard]]
        static constexpr bool has_slice() noexcept
        {
            return requires() { typename accessor_type::has_slice; };
        }
        [[nodiscard]]
        static constexpr bool has_custom_slice() noexcept
        {
            if constexpr(has_slice())
            {
                return requires(T object, slice s) { accessor_type::get(object, s); };
            }
            else
                return false;
        }

        [[noreturn]]
        constexpr static void index_unavailable()
        {
            throw std::runtime_error("index unavailable");
        }
        [[noreturn]]
        constexpr static void key_unavailable()
        {
            throw std::runtime_error("key unavailable");
        }
        [[noreturn]]
        constexpr static void slice_unavailable()
        {
            throw std::runtime_error("slice unavailable");
        }

        template <typename U>
        static format_arg get_arg(U&& object, const indexing_value& idx);

        template <typename U>
        static decltype(auto) get(U&& object, indexing_value::index_type i)
        {
            return index_handler(std::forward<U>(object), i);
        }
        template <typename U>
        static decltype(auto) get(U&& object, indexing_value::string_view_type str)
        {
            return index_handler(std::forward<U>(object), str);
        }
        template <typename U>
        static decltype(auto) get(U&& object, slice s)
        {
            return index_handler(std::forward<U>(object), s);
        }

        template <typename U>
        static format_arg get_attr(U&& object, const attribute_name& attr);

    private:
        template <typename U>
        static decltype(auto) index_handler(U&& object, indexing_value::index_type i)
        {
            if constexpr(!has_index())
            {
                index_unavailable();
            }
            else if constexpr(has_custom_index())
            {
                return accessor_type::get(std::forward<U>(object), i);
            }
            else
            {
                if(i < 0)
                    throw std::runtime_error("reverse index unavailable");
                return object[static_cast<std::size_t>(i)];
            }
        }
        template <typename U>
        static decltype(auto) index_handler(U&& object, indexing_value::string_view_type str)
        {
            if constexpr(!has_key())
            {
                key_unavailable();
            }
            else if constexpr(has_custom_key())
            {
                using Accessor = accessor<T>;
                return Accessor::get(std::forward<U>(object), str);
            }
            else
            {
                return object[str];
            }
        }
        template <typename U>
        static decltype(auto) index_handler(U&& object, slice s)
        {
            if constexpr(!has_slice())
            {
                slice_unavailable();
            }
            else if constexpr(has_custom_slice())
            {
                using Accessor = accessor<T>;
                return Accessor::get(std::forward<U>(object), s);
            }
            else
            {
                return object[s];
            }
        }
    };

    namespace detail
    {
        template <std::integral Integral>
        struct best_int_type
        {
            using type = std::conditional_t<
                std::is_unsigned_v<Integral>,
                std::conditional_t<(sizeof(Integral) <= sizeof(unsigned int)), unsigned int, unsigned long long int>,
                std::conditional_t<(sizeof(Integral) <= sizeof(int)), int, long long int>
            >;
        };

        template <std::integral Integral>
        using best_int_type_t = best_int_type<Integral>::type;

        class handle_impl_base
        {
        public:
            handle_impl_base() = default;
            handle_impl_base(const handle_impl_base&) = delete;

            virtual ~handle_impl_base() = default;

            virtual format_arg index(const indexing_value& idx) const = 0;
            virtual format_arg attribute(const attribute_name& attr) const = 0;

            virtual void format(format_spec_parse_context& spec, dynamic_format_context& ctx) const = 0;

            virtual void reset(handle_impl_base* mem) const = 0;
        };

        template <typename T>
        class handle_impl final : public handle_impl_base
        {
        public:
            handle_impl(const T& val) noexcept
                : m_ptr(&val) {}

            format_arg index(const indexing_value& idx) const override;
            format_arg attribute(const attribute_name& attr) const override;

            void format(format_spec_parse_context& spec, dynamic_format_context& ctx) const override;

            void reset(handle_impl_base* mem) const override
            {
                new(mem) handle_impl(*m_ptr);
            }

        private:
            const T* m_ptr;
        };
        // used for calculating storage space
        template <>
        class handle_impl<void> final : public handle_impl_base
        {
        public:
            handle_impl() = delete;

        private:
            const void* m_ptr;
        };

        template <typename T>
        concept integral_type =
            std::integral<T> &&
            !char_type<T>;

        template <typename T>
        concept use_handle =
            !std::is_same_v<T, utf8::codepoint> &&
            !char_type<T> &&
            !integral_type<T> &&
            !std::floating_point<T> &&
            !std::is_same_v<T, string_container> &&
            !string_like<T>;
    }

    class format_arg
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        class handle
        {
        public:
            handle() noexcept
                : m_storage{}, m_has_value(false) {}
            handle(const handle& other) noexcept
                : handle()
            {
                copy(other);
            }
            template <typename T>
            handle(const T& val)
                : handle()
            {
                construct<T>(val);
            }

            handle& operator=(const handle& rhs) noexcept
            {
                if(this == &rhs)
                    return *this;
                destroy();
                copy(rhs);
                return *this;
            }

            format_arg index(const indexing_value& idx) const
            {
                return ptr()->index(idx);
            }
            format_arg attribute(const attribute_name& attr) const
            {
                return ptr()->attribute(attr);
            }

            void format(format_spec_parse_context& spec, dynamic_format_context& ctx) const
            {
                ptr()->format(spec, ctx);
            }

        private:
            char m_storage[sizeof(detail::handle_impl<void>)]{};
            bool m_has_value = false;

            detail::handle_impl_base* ptr() noexcept
            {
                return reinterpret_cast<detail::handle_impl_base*>(m_storage);
            }
            const detail::handle_impl_base* ptr() const noexcept
            {
                return reinterpret_cast<const detail::handle_impl_base*>(m_storage);
            }

            template <typename T>
            void construct(const T& val) noexcept
            {
                static_assert(sizeof(detail::handle_impl<T>) <= sizeof(m_storage));
                new(ptr()) detail::handle_impl<T>(val);
                m_has_value = true;
            }
            void copy(const handle& other) noexcept;
            void destroy() noexcept;
        };

        using underlying_type = std::variant<
            std::monostate,
            bool,
            utf8::codepoint,
            int,
            unsigned int,
            long long int,
            unsigned long long int,
            float,
            double,
            long double,
            string_container,
            const void*,
            handle
        >;

        format_arg() noexcept
            : m_val() {}
        format_arg(const format_arg&) noexcept = default;
        format_arg(format_arg&&) noexcept = default;
        format_arg(bool val) noexcept
            : m_val(std::in_place_type<bool>, val) {}
        format_arg(utf8::codepoint cp) noexcept
            : m_val(std::in_place_type<utf8::codepoint>, cp) {}
        template <detail::char_type Char>
        format_arg(Char ch) noexcept
            : m_val(std::in_place_type<utf8::codepoint>, char32_t(ch)) {}
        template <detail::integral_type Integral>
        format_arg(Integral val) noexcept
            : m_val(static_cast<detail::best_int_type_t<Integral>>(val)) {}
        template <std::floating_point Float>
        format_arg(Float val) noexcept
            : m_val(val) {}
        format_arg(string_container str) noexcept
            : m_val(std::in_place_type<string_container>, std::move(str)) {}
        template <string_like String>
        format_arg(String&& str)
            : m_val(std::in_place_type<string_container>, std::forward<String>(str)) {}
        template <string_like String>
        format_arg(independent_t, String&& str)
            : m_val(std::in_place_type<string_container>, independent, std::forward<String>(str)) {}
        template <typename T, typename... Args>
        format_arg(std::in_place_type_t<T>, Args&&... args)
            : m_val(std::in_place_type<T>, std::forward<Args>(args)...) {}
        template <typename T> requires(!detail::char_type<T>)
        format_arg(const T* ptr) noexcept
            : m_val(std::in_place_type<const void*>, ptr) {}
        format_arg(std::nullptr_t) noexcept
            : m_val(std::in_place_type<const void*>, nullptr) {}
        template <detail::use_handle T>
        format_arg(const T& val) noexcept
            : m_val(std::in_place_type<handle>, val) {}

        format_arg& operator=(const format_arg&) = default;
        format_arg& operator=(format_arg&&) noexcept = default;

        [[nodiscard]]
        format_arg index(const indexing_value& idx) const;
        [[nodiscard]]
        format_arg attribute(const attribute_name& attr) const;

        template <typename T>
        [[nodiscard]]
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_val);
        }
        [[nodiscard]]
        bool empty() const noexcept
        {
            return holds<std::monostate>();
        }

        template <typename T>
        [[nodiscard]]
        friend const auto& get(const format_arg& val)
        {
            if constexpr(detail::char_type<T>)
                return std::get<utf8::codepoint>(val.m_val);
            else if constexpr(std::integral<T>)
                return std::get<detail::best_int_type_t<T>>(val.m_val);
            else if constexpr(std::is_same_v<T, string_container> || string_like<T>)
                return std::get<string_container>(val.m_val);
            else
                return std::get<T>(val.m_val);
        }

        script::variable as_variable() const;

        template <typename Context>
        void format(format_spec_parse_context& spec, Context& ctx);

    private:
        underlying_type m_val;
    };

    // access members of format argument
    class format_arg_access
    {
    public:
        using member_type = std::variant<
            indexing_value,
            attribute_name
        >;
        using member_storage = small_vector<member_type, 2>;

        format_arg_access() noexcept : m_members() {}
        format_arg_access(const format_arg_access&) = delete;
        format_arg_access(format_arg_access&&) noexcept = default;
        format_arg_access(member_storage members)
            : m_members(std::move(members)) {}

        [[nodiscard]]
        format_arg access(format_arg arg) const;

        [[nodiscard]]
        bool empty() const noexcept
        {
            return m_members.empty();
        }

    private:
        member_storage m_members;
    };

    class dynamic_format_arg_store
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using size_type = std::size_t;

        dynamic_format_arg_store() = default;
        dynamic_format_arg_store(const dynamic_format_arg_store&) = delete;
        dynamic_format_arg_store(dynamic_format_arg_store&&) = default;
        template <typename... Args>
        dynamic_format_arg_store(Args&&... args)
        {
            emplace(std::forward<Args>(args)...);
        }

        template <typename... Args>
        void emplace(Args&&... args)
        {
            auto helper = [&]<typename T> (T&& arg)
            {
                if constexpr(requires() { typename T::named_arg_tag; })
                {
                    m_named_args.emplace(std::make_pair(arg.name, std::forward<T>(arg).value));
                }
                else
                {
                    m_args.emplace_back(std::forward<T>(arg));
                }
            };

            (helper(std::forward<Args>(args)), ...);
        }

        [[nodiscard]]
        const format_arg& get(size_type i) const;
        [[nodiscard]]
        const format_arg& get(string_view_type key) const;
        [[nodiscard]]
        const format_arg& get(const indexing_value& idx) const;

        [[nodiscard]]
        bool check(const indexing_value& idx) const noexcept;

        [[nodiscard]]
        const format_arg& operator[](const indexing_value& idx) const
        {
            return get(idx);
        }

        [[nodiscard]]
        size_type size() const noexcept
        {
            return m_args.size();
        }
        [[nodiscard]]
        size_type named_size() const noexcept
        {
            return m_named_args.size();
        }

        void clear() noexcept
        {
            m_args.clear();
            m_named_args.clear();
        }

    private:
        std::vector<format_arg> m_args;
        std::map<string_type, format_arg, std::less<>> m_named_args;
    };

    // WARNING: This class only holds the view of string and reference of arguments
    // The invoker needs to handle lifetimes manually
    class format_parse_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = string_view_type::iterator;

        format_parse_context() = delete;
        format_parse_context(const format_parse_context&) = delete;
        format_parse_context(string_view_type str, const dynamic_format_arg_store& store);

        const dynamic_format_arg_store& get_store() const noexcept
        {
            return *m_store;
        }

        void enable_manual_indexing() noexcept
        {
            m_manual_indexing = true;
        }
        void advance_to(iterator it) noexcept
        {
            m_it = it;
        }

        iterator begin() const noexcept
        {
            return m_it;
        }
        iterator end() const noexcept
        {
            return m_view.end();
        }

        std::size_t current_arg_id() const
        {
            if(m_manual_indexing)
                invalid_default_argument();
            return m_default_arg_idx;
        }
        std::size_t next_arg_id()
        {
            if(m_manual_indexing)
                invalid_default_argument();
            return ++m_default_arg_idx;
        }
        std::size_t check_arg_id(std::size_t i) const
        {
            enable_manual_indexing();
            return get_store().check(i);
        }

        [[nodiscard]]
        bool manual_indexing() const noexcept
        {
            return m_manual_indexing;
        }

        [[noreturn]]
        static void invalid_default_argument()
        {
            throw std::runtime_error("no default argument after an explicit argument");
        }

    private:
        string_view_type m_view;
        iterator m_it;
        mutable bool m_manual_indexing = false;
        std::size_t m_default_arg_idx = 0;
        const dynamic_format_arg_store* m_store;

        void enable_manual_indexing() const noexcept
        {
            m_manual_indexing = true;
        }
    };

    // WARNING: This class only holds the view of string and reference of arguments
    // The invoker needs to handle lifetimes manually
    class format_spec_parse_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = string_view_type::const_iterator;
        using reverse_iterator = string_view_type::const_reverse_iterator;

        format_spec_parse_context() = delete;
        format_spec_parse_context(const format_spec_parse_context&) = delete;
        constexpr format_spec_parse_context(string_view_type str, const dynamic_format_arg_store& store) noexcept
            : m_base(nullptr), m_spec_str(str), m_store(&store) {}
        format_spec_parse_context(format_parse_context& base, string_view_type str) noexcept
            : m_base(&base), m_spec_str(str), m_store(&base.get_store()) {}

        [[nodiscard]]
        constexpr bool has_base() const noexcept
        {
            return m_base != nullptr;
        }
        format_parse_context& get_base() const noexcept
        {
            return *m_base;
        }

        std::size_t current_arg_id() const
        {
            return get_base().current_arg_id();
        }
        std::size_t next_arg_id()
        {
            return get_base().next_arg_id();
        }
        std::size_t check_arg_id(std::size_t i) const
        {
            return get_base().check_arg_id(i);
        }

        [[nodiscard]]
        bool manual_indexing() const noexcept
        {
            return get_base().manual_indexing();
        }

        [[nodiscard]]
        constexpr iterator begin() const noexcept
        {
            return m_spec_str.begin();
        }
        [[nodiscard]]
        constexpr iterator end() const noexcept
        {
            return m_spec_str.end();
        }
        [[nodiscard]]
        constexpr reverse_iterator rbegin() const noexcept
        {
            return m_spec_str.rbegin();
        }
        [[nodiscard]]
        constexpr reverse_iterator rend() const noexcept
        {
            return m_spec_str.rend();
        }

        [[nodiscard]]
        constexpr operator string_view_type() const noexcept
        {
            return m_spec_str;
        }

        [[nodiscard]]
        constexpr bool has_store() const noexcept
        {
            return m_store != nullptr;
        }
        [[nodiscard]]
        constexpr const dynamic_format_arg_store& get_store() const noexcept
        {
            return *m_store;
        }

    private:
        format_parse_context* m_base;
        string_view_type m_spec_str;
        const dynamic_format_arg_store* m_store;
    };

    template <typename OutputIt, typename Store = dynamic_format_arg_store>
    class basic_format_context
    {
    public:
        using char_type = char;
        using iterator = OutputIt;
        using store_type = Store;

        basic_format_context(iterator it, const store_type& store)
            : m_out(std::move(it)), m_store(&store) {}

        iterator out()
        {
            return m_out;
        }
        void advance_to(iterator it)
        {
            m_out = std::move(it);
        }

        const store_type& get_store() const noexcept
        {
            return *m_store;
        }

    private:
        iterator m_out;
        const store_type* m_store;
    };

    namespace detail
    {
        class dynamic_format_context_impl_base
        {
        public:
            using char_type = char;

            virtual void push_back(char_type ch) = 0;
            virtual const dynamic_format_arg_store& get_store() const noexcept = 0;
        };
        template <typename OutputIt, typename Store>
        class dynamic_format_context_impl final : public dynamic_format_context_impl_base
        {
        public:
            using iterator = OutputIt;
            using store_type = Store;

            dynamic_format_context_impl(basic_format_context<OutputIt, Store>& ctx) noexcept
                : m_out(ctx.out()), m_store(&ctx.get_store()) {}

            void push_back(char_type ch) override
            {
                *m_out = ch;
                ++m_out;
            }
            const dynamic_format_arg_store& get_store() const noexcept override
            {
                return *m_store;
            }
        private:
            iterator m_out;
            const dynamic_format_arg_store* m_store;
        };
    }

    class dynamic_format_context
    {
    public:
        using char_type = char;
        using iterator = std::back_insert_iterator<dynamic_format_context>;
        using store_type = dynamic_format_arg_store;
        using value_type = char_type;

        dynamic_format_context() = delete;
        template <typename OutputIt, typename Store>
        dynamic_format_context(basic_format_context<OutputIt, Store>& ctx)
            : m_impl(std::make_unique<detail::dynamic_format_context_impl<OutputIt, Store>>(ctx)) {}

        iterator out()
        {
            return std::back_inserter(*this);
        }
        void advance_to(iterator) {}

        const store_type& get_store() const noexcept
        {
            return m_impl->get_store();
        }

        void push_back(char_type ch)
        {
            m_impl->push_back(ch);
        }

    private:
        std::unique_ptr<detail::dynamic_format_context_impl_base> m_impl;
    };

    template <typename Context>
    class format_context_traits
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = typename Context::iterator;
        using store_type = typename Context::store_type;

        format_context_traits() = delete;
        format_context_traits(const format_context_traits&) noexcept = default;
        format_context_traits(Context& ctx) noexcept
            : m_ctx(&ctx) {}

        iterator out()
        {
            return m_ctx->out();
        }
        void advance_to(iterator it)
        {
            m_ctx->advance_to(std::move(it));
        }

        const store_type& get_store() const noexcept
        {
            return m_ctx->get_store();
        }

        template <typename InputIt>
        void append(InputIt begin, InputIt end)
        {
            advance_to(std::copy(begin, end, out()));
        }
        void append(string_view_type str)
        {
            append(str.begin(), str.end());
        }
        template <detail::char_type Char>
        void append(Char ch, std::size_t count = 1)
        {
            if constexpr(std::is_same_v<Char, char> || std::is_same_v<Char, char8_t>)
            {
                advance_to(std::fill_n(out(), count, static_cast<char>(ch)));
            }
            else
            {
                utf8::codepoint cp(static_cast<char32_t>(ch));
                append(cp, count);
            }
        }
        void append(utf8::codepoint cp, std::size_t count = 1)
        {
            for(std::size_t i = 0; i < count; ++i)
                append(string_view_type(cp));
        }

    private:
        Context* m_ctx;
    };
}

#include "core.inl"
