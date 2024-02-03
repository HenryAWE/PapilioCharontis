#include <papilio/container.hpp>
#include <stdexcept>

namespace papilio
{
namespace detail
{
    void small_vector_impl::throw_out_of_range()
    {
        throw std::out_of_range("small vector index out of range");
    }

    void small_vector_impl::throw_length_error()
    {
        throw std::length_error("length error");
    }

    auto small_vector_impl::get_mem_size(
        size_type current, size_type required
    ) noexcept -> size_type
    {
        size_type new_size = current + current / 2;
        if(new_size < required)
            new_size = required;

        return new_size;
    }

    void fixed_vector_impl::throw_out_of_range()
    {
        throw std::out_of_range("out of range");
    }

    void fixed_vector_impl::throw_length_error()
    {
        throw std::length_error("length error");
    }

    void fixed_flat_map_impl::throw_out_of_range()
    {
        throw std::out_of_range("out of range");
    }
} // namespace detail
} // namespace papilio
