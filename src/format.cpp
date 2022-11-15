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
