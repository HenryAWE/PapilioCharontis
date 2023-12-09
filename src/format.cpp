#include <papilio/format.hpp>

namespace papilio
{
std::string vformat(std::string_view fmt, const dynamic_format_args& store)
{
    std::string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), fmt, store);

    return result;
}

std::string vformat(const std::locale& loc, std::string_view fmt, const dynamic_format_args& store)
{
    std::string result;
    PAPILIO_NS vformat_to(std::back_inserter(result), loc, fmt, store);

    return result;
}

namespace detail
{
    struct formatted_size_counter
    {
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        std::size_t value = 0;

        formatted_size_counter() noexcept = default;
        formatted_size_counter(const formatted_size_counter&) noexcept = default;

        formatted_size_counter& operator=(const formatted_size_counter&) noexcept = default;

        formatted_size_counter& operator=(char) noexcept
        {
            return *this;
        }

        formatted_size_counter& operator*() noexcept
        {
            return *this;
        }

        formatted_size_counter& operator++() noexcept
        {
            ++value;
            return *this;
        }

        formatted_size_counter operator++(int) noexcept
        {
            formatted_size_counter tmp(*this);
            ++(*this);
            return tmp;
        }
    };
} // namespace detail

std::size_t vformatted_size(std::string_view fmt, const dynamic_format_args& store)
{
    auto result = PAPILIO_NS vformat_to(detail::formatted_size_counter(), fmt, store);
    return result.value;
}

std::size_t vformatted_size(const std::locale& loc, std::string_view fmt, const dynamic_format_args& store)
{
    auto result = PAPILIO_NS vformat_to(detail::formatted_size_counter(), loc, fmt, store);
    return result.value;
}
} // namespace papilio
