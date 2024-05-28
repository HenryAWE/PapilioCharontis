#include "os_impl.hpp"
#include <papilio/detail/prefix.hpp>

#ifdef PAPILIO_OS_IMPL_POSIX

#    include <unistd.h>

namespace papilio::os
{
bool is_terminal(std::FILE* file)
{
    return isatty(fileno(file)) != 0;
}

void output_conv(
    std::FILE* file,
    std::string_view out
)
{
    output_nonconv(file, out);
}
} // namespace papilio::os

#endif

#include <papilio/detail/suffix.hpp>
