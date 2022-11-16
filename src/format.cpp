#include <papilio/format.hpp>


namespace papilio
{
    std::string vformat(std::string_view fmt, const dynamic_format_arg_store& store)
    {
        std::string result;
        vformat_to(std::back_inserter(result), fmt, store);

        return result;
    }
    std::string vformat(const std::locale& loc, std::string_view fmt, const dynamic_format_arg_store& store)
    {
        std::string result;
        vformat_to(std::back_inserter(result), loc, fmt, store);

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
            formatted_size_counter& operator=(char) noexcept { return *this; }

            formatted_size_counter& operator*() noexcept { return *this; }
            formatted_size_counter& operator++() noexcept { ++value; return *this; }
            formatted_size_counter operator++(int) noexcept
            {
                formatted_size_counter tmp(*this);
                ++(*this);
                return tmp;
            }
        };
    }

    std::size_t vformatted_size(std::string_view fmt, const dynamic_format_arg_store& store)
    {
        auto result = vformat_to(detail::formatted_size_counter(), fmt, store);
        return result.value;
    }
    std::size_t vformatted_size(const std::locale& loc, std::string_view fmt, const dynamic_format_arg_store& store)
    {
        auto result = vformat_to(detail::formatted_size_counter(), loc, fmt, store);
        return result.value;
    }

    namespace detail
    {
        std::pair<std::size_t, std::size_t> calc_fill_width(format_align align, std::size_t width, std::size_t current)
        {
            if(width <= current)
                return std::make_pair(0, 0);

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            std::size_t to_fill = width - current;

            using enum format_align;
            switch(align)
            {
            case left:
                fill_back = to_fill;
                break;

            [[likely]] case right:
                fill_front = to_fill;
                break;

            case middle:
                fill_front = to_fill / 2; // floor(to_fill / 2)
                fill_back = to_fill / 2 + to_fill % 2; // ceil(to_fill / 2)
                break;
            }

            return std::make_pair(fill_front, fill_back);
        }
    }
}
