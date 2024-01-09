#include <papilio/print.hpp>
#include <system_error>
#ifdef PAPILIO_PLATFORM_WINDOWS
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#endif

namespace papilio
{
namespace detail
{
    void cfile_iterator::write(char ch)
    {
        int result = std::fputc(ch, m_file);
        if(result == EOF)
        {
            throw std::system_error(
                std::make_error_code(std::errc::io_error)
            );
        }
    }

    void cfile_iterator_conv::write(char ch)
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
#ifndef PAPILIO_PLATFORM_WINDOWS
            m_underlying = std::copy(
                m_buf, m_buf + m_byte_len, m_underlying
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
                static_cast<int>(std::size(mbbuf)),
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
    }

    unsigned int get_output_cp_win() noexcept
    {
#ifdef PAPILIO_PLATFORM_WINDOWS
        return ::GetConsoleCP();
#else
        return 0;
#endif
    }
} // namespace detail

void println(std::FILE* file)
{
    detail::cfile_iterator it(file);
    *it = '\n';
}

void println()
{
    println(stdout);
}

void println(std::ostream& os)
{
    os << '\n';
}
} // namespace papilio
