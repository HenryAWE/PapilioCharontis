#include <papilio/detail/container.hpp>
#include <cassert>
#include <stdexcept>


namespace papilio::detail
{
    void small_vector_base::raise_out_of_range()
    {
        throw std::out_of_range("small vector index out of range");
    }
    [[noreturn]]
    void small_vector_base::raise_length_error()
    {
        throw std::length_error("length error");
    }

    small_vector_base::size_type small_vector_base::calc_mem_size(size_type current, size_type required) noexcept
    {
        assert(current < required);
        if(current <= 1)
            current = 2;
        while(current < required)
            current += current / 2;
        return current;
    }

    void fixed_flat_map_base::raise_out_of_range()
    {
        throw std::out_of_range("out of range");
    }
}
