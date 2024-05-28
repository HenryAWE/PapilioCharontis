#include "os_impl.hpp"
#include <papilio/detail/prefix.hpp>

namespace papilio::os
{
void output_nonconv(
    std::FILE* file,
    std::string_view out
)
{
    using namespace std;

    size_t result = fwrite(
        out.data(), 1, out.size(), file
    );
    if(result < out.size())
    {
        throw std::system_error(
            std::make_error_code(std::errc::io_error)
        );
    }

    fflush(file);
}
} // namespace papilio::os

#include <papilio/detail/suffix.hpp>
