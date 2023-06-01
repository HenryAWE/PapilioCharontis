#include <papilio/container.hpp>
#include <cassert>
#include <stdexcept>


namespace papilio
{
    namespace detail
    {
        void small_vector_impl_base::raise_out_of_range()
        {
            throw std::out_of_range("small vector index out of range");
        }
        [[noreturn]]
        void small_vector_impl_base::raise_length_error()
        {
            throw std::length_error("length error");
        }

        small_vector_impl_base::size_type small_vector_impl_base::calc_mem_size(size_type current, size_type required) noexcept
        {
            assert(current < required);
            if(current <= 1)
                current = 2;
            while(current < required)
                current += current / 2;
            return current;
        }
    }

    namespace detail
    {
        void fixed_vector_impl_base::raise_out_of_range()
        {
            throw std::out_of_range("out of range");
        }
        void fixed_vector_impl_base::raise_length_error()
        {
            throw std::length_error("length error");
        }
    }

    namespace detail
    {
        void fixed_flat_map_impl_base::raise_out_of_range()
        {
            throw std::out_of_range("out of range");
        }
    }
}
