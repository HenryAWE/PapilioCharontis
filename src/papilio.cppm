module;
#include <papilio/papilio.hpp>


export module papilio;
export import <string>;
export import <string_view>;
export import <variant>;
export import <iostream>;

export namespace papilio
{
    const char* library_info() noexcept
    {
        return "Papilio Charontis v0.1.0";
    }

    template <typename... Args>
    void format_to(std::ostream& os, std::string_view fmt, Args&&... args)
    {
        auto result = vformat(fmt, make_format_args(std::forward<Args>(args)...));
        os << result;
    }
}
