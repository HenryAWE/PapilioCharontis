#include <papilio/papilio.hpp>


namespace papilio
{
    std::tuple<int, int, int> get_version() noexcept
    {
        return std::make_tuple(
            version_major,
            version_minor,
            version_patch
        );
    }
}
