#pragma once

#include <cstdio> // FILE*
#include <iostream>
#include <iterator>
#include "macros.hpp"
#include "format.hpp"
#include "color.hpp"

namespace papilio
{
namespace detail
{
    // output iterator for C FILE*
    class fp_iterator
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        fp_iterator() = delete;
        fp_iterator(const fp_iterator&) noexcept = default;

        fp_iterator(std::FILE* fp) noexcept
            : m_fp(fp) {}

        fp_iterator& operator=(const fp_iterator&) noexcept = default;

        fp_iterator& operator=(char ch)
        {
            write(ch);
            return *this;
        }

        fp_iterator& operator*() noexcept
        {
            return *this;
        }

        fp_iterator& operator++() noexcept
        {
            return *this;
        }

        fp_iterator operator++(int) noexcept
        {
            return *this;
        }

        std::FILE* get() const noexcept
        {
            return m_fp;
        }

    private:
        std::FILE* m_fp;

        void write(char ch);
    };

    class fp_iterator_conv_base
    {
    public:
        fp_iterator_conv_base() noexcept = default;
        fp_iterator_conv_base(const fp_iterator_conv_base&) noexcept = default;

#ifdef PAPILIO_PLATFORM_WINDOWS

    protected:
        fp_iterator_conv_base(unsigned int win_cp) noexcept
            : m_win_cp(win_cp) {}

        unsigned int get_cp() const noexcept
        {
            return m_win_cp;
        }

    private:
        unsigned int m_win_cp = 0; // 0 == CP_ACP
#else

    protected:
        fp_iterator_conv_base([[maybe_unused]] unsigned int win_cp) noexcept {}

        unsigned int get_cp() const noexcept = delete;
#endif
    };

    class fp_iterator_conv : public fp_iterator_conv_base
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        fp_iterator_conv() = delete;
        fp_iterator_conv(const fp_iterator_conv&) noexcept = default;

        fp_iterator_conv(std::FILE* fp, int win_cp = 0) noexcept
            : fp_iterator_conv_base(win_cp), m_underlying(fp) {}

        fp_iterator_conv& operator=(const fp_iterator_conv&) noexcept = default;

        fp_iterator_conv& operator=(char ch)
        {
            write(ch);
            return *this;
        }

        fp_iterator_conv& operator*() noexcept
        {
            return *this;
        }

        fp_iterator_conv& operator++() noexcept
        {
            return *this;
        }

        fp_iterator_conv operator++(int) noexcept
        {
            return *this;
        }

    private:
        char8_t m_buf[4] = {u8'\0', u8'\0', u8'\0', u8'\0'};
        std::uint8_t m_byte_len = 0;
        std::uint8_t m_byte_idx = 0;
        fp_iterator m_underlying;

        void write(char ch);
    };

    unsigned int get_output_cp_win() noexcept;

    inline auto create_iter(std::FILE* file) noexcept
    {
#ifdef PAPILIO_PLATFORM_WINDOWS
        return fp_iterator_conv(file, get_output_cp_win());
#else
        return fp_iterator(file);
#endif
    }
} // namespace detail

void println(std::FILE* file);
void println();

template <typename... Args>
void print(std::FILE* file, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::fp_iterator;
    using context_type = basic_format_context<iter_t>;
    PAPILIO_NS vformat_to(
        iter_t(file),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

template <typename... Args>
void print(format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::fp_iterator_conv;
    using context_type = basic_format_context<iter_t>;
    PAPILIO_NS vformat_to(
        iter_t(stdout, detail::get_output_cp_win()),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

template <typename... Args>
void print(text_style st, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::fp_iterator_conv;
    using context_type = basic_format_context<iter_t>;

    auto it = iter_t(stdout, detail::get_output_cp_win());

    it = st.set(it);

    it = PAPILIO_NS vformat_to(
        it,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );

    st.reset(it);
}

template <typename... Args>
void println(std::FILE* file, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::fp_iterator;
    using context_type = basic_format_context<iter_t>;
    auto it = PAPILIO_NS vformat_to(
        iter_t(file),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
    *it = '\n';
}

template <typename... Args>
void println(format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::fp_iterator_conv;
    using context_type = basic_format_context<iter_t>;
    auto it = vformat_to(
        iter_t(stdout, detail::get_output_cp_win()),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
    *it = '\n';
}

template <typename... Args>
void println(text_style st, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::fp_iterator_conv;
    using context_type = basic_format_context<iter_t>;

    auto it = iter_t(stdout, detail::get_output_cp_win());

    it = st.set(it);

    it = PAPILIO_NS vformat_to(
        it,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );

    it = st.reset(it);

    *it = '\n';
}

template <typename... Args>
void print(std::ostream& os, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = std::ostream_iterator<char>;
    using context_type = basic_format_context<iter_t>;
    PAPILIO_NS vformat_to(
        iter_t(os),
        os.getloc(),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

void println(std::ostream& os);

template <typename... Args>
void println(std::ostream& os, format_string<Args...> fmt, Args&&... args)
{
    PAPILIO_NS print(os, fmt.get(), std::forward<Args>(args)...);
    PAPILIO_NS println(os);
}
} // namespace papilio
