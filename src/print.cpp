#include <papilio/print.hpp>
#include <system_error>
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif
#include <iostream>
#include <iterator>


namespace papilio
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
            int result = std::fputc(ch, m_file);
            if(result == EOF)
            {
                throw std::system_error(
                    std::make_error_code(std::errc::io_error)
                );
            }
            return *this;
        }

        cfile_iterator& operator*() noexcept
        {
            return *this;
        }
        cfile_iterator& operator++() noexcept { return *this; }
        cfile_iterator operator++(int) noexcept { return *this; }

        std::FILE* get() const noexcept
        {
            return m_file;
        }

    private:
        std::FILE* m_file;
    };

    class cfile_iterator_utf8_base
    {
    public:
        cfile_iterator_utf8_base() noexcept = default;
        cfile_iterator_utf8_base(const cfile_iterator_utf8_base&) noexcept = default;

#ifdef _WIN32
    protected:
        cfile_iterator_utf8_base(unsigned int win_cp) noexcept
            : m_win_cp(win_cp) {}

        unsigned int get_cp() const noexcept
        {
            return m_win_cp;
        }

    private:
        ::UINT m_win_cp = CP_ACP;
#else
    protected:
        cfile_iterator_utf8_base(unsigned int win_cp) noexcept {}

        unsigned int get_cp() const noexcept = delete;
#endif
    };
    class cfile_iterator_utf8 : public cfile_iterator_utf8_base
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        cfile_iterator_utf8() = delete;
        cfile_iterator_utf8(const cfile_iterator_utf8&) noexcept = default;
        cfile_iterator_utf8(std::FILE* file, int win_cp = 0) noexcept
            : m_underlying(file), cfile_iterator_utf8_base(win_cp) {}

        cfile_iterator_utf8& operator=(const cfile_iterator_utf8&) noexcept = default;
        cfile_iterator_utf8& operator=(char ch)
        {
            if(m_byte_len == 0)
            {
                m_byte_len = utf::is_leading_byte(ch);
                if(m_byte_len == 0) // invalid UTF-8 data
                {
                    throw std::system_error(
                        std::make_error_code(std::errc::io_error)
                    );
                }
            }

            m_buf[m_byte_idx] = static_cast<char8_t>(ch);
            ++m_byte_idx;

            if(m_byte_idx == m_byte_len)
            {
#ifndef _WIN32
                m_underlying = std::copy(
                    m_buf, m_buf + m_byte_len,
                    m_underlying
                );
#else
                wchar_t wbuf[2]{};
                int wconv_result = ::MultiByteToWideChar(
                    CP_UTF8,
                    MB_ERR_INVALID_CHARS,
                    (::LPSTR)m_buf,
                    m_byte_len,
                    wbuf,
                    2
                );
                if(wconv_result == 0)
                {
                    throw std::system_error(
                        std::make_error_code(std::errc::io_error)
                    );
                }

                char mbbuf[8]{};
                int mbconv_result = ::WideCharToMultiByte(
                    get_cp(),
                    0,
                    wbuf,
                    wconv_result,
                    mbbuf,
                    std::size(mbbuf),
                    nullptr,
                    nullptr
                );
                if(mbconv_result == 0)
                {
                    throw std::system_error(
                        std::make_error_code(std::errc::io_error)
                    );
                }

                m_underlying = std::copy_n(
                    mbbuf,
                    mbconv_result,
                    m_underlying
                );
#endif

                m_byte_len = 0;
                m_byte_idx = 0;
            }
            return *this;
        }

        cfile_iterator_utf8& operator*() noexcept
        {
            return *this;
        }
        cfile_iterator_utf8& operator++() noexcept { return *this; }
        cfile_iterator_utf8 operator++(int) noexcept { return *this; }

    private:
        char8_t m_buf[4] = { u8'\0', u8'\0', u8'\0', u8'\0' };
        std::uint8_t m_byte_len = 0;
        std::uint8_t m_byte_idx = 0;
        cfile_iterator m_underlying;
    };

    void vprint(std::FILE* file, std::string_view fmt, const dynamic_format_args& args)
    {
        PAPILIO_NS vformat_to(cfile_iterator(file), fmt, args);
    }
    void vprintln(std::FILE* file, std::string_view fmt, const dynamic_format_args& args)
    {
        auto it = PAPILIO_NS vformat_to(cfile_iterator(file), fmt, args);
        *it = '\n';
    }
    void vprint_conv(std::FILE* file, std::string_view fmt, const dynamic_format_args& args)
    {
        PAPILIO_NS vformat_to(cfile_iterator_utf8(file), fmt, args);
    }
    void vprintln_conv(std::FILE* file, std::string_view fmt, const dynamic_format_args& args)
    {
        auto it = PAPILIO_NS vformat_to(cfile_iterator_utf8(file), fmt, args);
        *it = '\n';
    }

    static unsigned int get_output_cp_win() noexcept
    {
#ifdef _WIN32
        return ::GetConsoleCP();
#else
        return 0;
#endif
    }
    void vprint_conv(std::string_view fmt, const dynamic_format_args& args)
    {
        PAPILIO_NS vformat_to(cfile_iterator_utf8(stdout, get_output_cp_win()), fmt, args);
    }
    void vprintln_conv(std::string_view fmt, const dynamic_format_args& args)
    {
        auto it = PAPILIO_NS  vformat_to(cfile_iterator_utf8(stdout, get_output_cp_win()), fmt, args);
        *it = '\n';
    }

    void println(std::FILE* file)
    {
        cfile_iterator it(file);
        *it = '\n';
    }
    void println()
    {
        println(stdout);
    }

    void vprint(std::ostream& os, std::string_view fmt, const dynamic_format_args& args)
    {
        vformat_to(std::ostream_iterator<char>(os), os.getloc(), fmt, args);
    }

    void println(std::ostream& os)
    {
        os << '\n';
    }
}
