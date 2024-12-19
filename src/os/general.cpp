#include "os_impl.hpp"
#include <cstdio>
#include <papilio/detail/prefix.hpp>

namespace papilio::os
{
void output_nonconv(
    std::FILE* file,
    std::string_view out
)
{
    std::size_t result = std::fwrite(
        out.data(), 1, out.size(), file
    );
    if(result < out.size())
    {
        throw std::system_error(
            std::make_error_code(std::errc::io_error)
        );
    }

    std::fflush(file);
}
} // namespace papilio::os

#include <papilio/detail/suffix.hpp>
