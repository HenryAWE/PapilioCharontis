#pragma once

#include <cstdio> // FILE*
#include <iostream>
#include <iterator>
#include "macros.hpp"
#include "format.hpp"

namespace papilio
{
namespace detail
{
    // output iterator for C FILE*
    class cfile_iterator
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        cfile_iterator() = delete;
        cfile_iterator(const cfile_iterator&) noexcept = default;

        cfile_iterator(std::FILE* file) noexcept
            : m_file(file) {}

        cfile_iterator& operator=(const cfile_iterator&) noexcept = default;

        cfile_iterator& operator=(char ch)
        {
            write(ch);
            return *this;
        }

        cfile_iterator& operator*() noexcept
        {
            return *this;
        }

        cfile_iterator& operator++() noexcept
        {
            return *this;
        }

        cfile_iterator operator++(int) noexcept
        {
            return *this;
        }

        std::FILE* get() const noexcept
        {
            return m_file;
        }

    private:
        std::FILE* m_file;

        void write(char ch);
    };

    class cfile_iterator_conv_base
    {
    public:
        cfile_iterator_conv_base() noexcept = default;
        cfile_iterator_conv_base(const cfile_iterator_conv_base&) noexcept = default;

#ifdef PAPILIO_PLATFORM_WINDOWS

    protected:
        cfile_iterator_conv_base(unsigned int win_cp) noexcept
            : m_win_cp(win_cp) {}

        unsigned int get_cp() const noexcept
        {
            return m_win_cp;
        }

    private:
        unsigned int m_win_cp = 0; // 0 == CP_ACP
#else

    protected:
        cfile_iterator_conv_base([[maybe_unused]] unsigned int win_cp) noexcept {}

        unsigned int get_cp() const noexcept = delete;
#endif
    };

    class cfile_iterator_conv : public cfile_iterator_conv_base
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        cfile_iterator_conv() = delete;
        cfile_iterator_conv(const cfile_iterator_conv&) noexcept = default;

        cfile_iterator_conv(std::FILE* file, int win_cp = 0) noexcept
            : cfile_iterator_conv_base(win_cp), m_underlying(file) {}

        cfile_iterator_conv& operator=(const cfile_iterator_conv&) noexcept = default;

        cfile_iterator_conv& operator=(char ch)
        {
            write(ch);
            return *this;
        }

        cfile_iterator_conv& operator*() noexcept
        {
            return *this;
        }

        cfile_iterator_conv& operator++() noexcept
        {
            return *this;
        }

        cfile_iterator_conv operator++(int) noexcept
        {
            return *this;
        }

    private:
        char8_t m_buf[4] = {u8'\0', u8'\0', u8'\0', u8'\0'};
        std::uint8_t m_byte_len = 0;
        std::uint8_t m_byte_idx = 0;
        cfile_iterator m_underlying;

        void write(char ch);
    };

    unsigned int get_output_cp_win() noexcept;

    inline auto create_iter(std::FILE* file) noexcept
    {
#ifdef PAPILIO_PLATFORM_WINDOWS
        return cfile_iterator_conv(file, get_output_cp_win());
#else
        return cfile_iterator(file);
#endif
    }
} // namespace detail

void println(std::FILE* file);
void println();

template <typename... Args>
void print(std::FILE* file, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::cfile_iterator;
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
    using iter_t = detail::cfile_iterator_conv;
    using context_type = basic_format_context<iter_t>;
    PAPILIO_NS vformat_to(
        iter_t(stdout, detail::get_output_cp_win()),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

template <typename... Args>
void println(std::FILE* file, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = detail::cfile_iterator;
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
    using iter_t = detail::cfile_iterator_conv;
    using context_type = basic_format_context<iter_t>;
    auto it = vformat_to(
        iter_t(stdout, detail::get_output_cp_win()),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
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
