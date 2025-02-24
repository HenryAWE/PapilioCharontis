#include <papilio/formatter/filesystem.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
namespace detail
{
    template <>
    utf::string_container path_to_sc<char>(const std::filesystem::path& p, bool gen)
    {
#ifndef PAPILIO_PLATFORM_WINDOWS
        static_assert(std::same_as<std::filesystem::path::value_type, char>);

        if(gen)
            return p.generic_string();
        else
            return p.native();

#else
        static_assert(std::same_as<std::filesystem::path::value_type, wchar_t>);

        utf::wstring_container wsc;
        if(gen)
            wsc = p.generic_wstring();
        else
            wsc = p.native();
        return wsc.to_string();
#endif
    }

    template <>
    utf::wstring_container path_to_sc<wchar_t>(const std::filesystem::path& p, bool gen)
    {
#ifndef PAPILIO_PLATFORM_WINDOWS
        static_assert(std::same_as<std::filesystem::path::value_type, char>);

        if(gen)
            return p.generic_wstring();
        else
            return p.wstring();

#else
        static_assert(std::same_as<std::filesystem::path::value_type, wchar_t>);

        if(gen)
            return p.generic_wstring();
        else
            return p.native();
#endif
    }
} // namespace detail
} // namespace papilio

#include <papilio/detail/suffix.hpp>
