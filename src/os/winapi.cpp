#include "os_impl.hpp"
#include <papilio/utf/string.hpp>
#include <papilio/detail/prefix.hpp>

#ifdef PAPILIO_OS_IMPL_WINAPI

#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    include <io.h>

namespace papilio::os
{
static HANDLE get_file_handle(std::FILE* file)
{
    return reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
}

bool is_terminal(std::FILE* file)
{
    DWORD mode = 0;
    GetConsoleMode(
        get_file_handle(file),
        &mode
    );
    return mode != 0;
}

void output_conv(
    std::FILE* file,
    std::string_view out
)
{
    if(is_terminal(file))
    {
        auto wout = utf::string_ref(out).to_wstring();

        BOOL result = WriteConsoleW(
            get_file_handle(file),
            wout.data(),
            static_cast<DWORD>(wout.size()),
            nullptr,
            nullptr
        );
        if(!result)
        {
            throw std::system_error(
                std::make_error_code(std::errc::io_error)
            );
        }
    }
    else
    {
        output_nonconv(file, out);
    }
}
} // namespace papilio::os

#endif

#include <papilio/detail/suffix.hpp>
