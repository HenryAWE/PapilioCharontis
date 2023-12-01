#include <papilio/container.hpp>
#include <cassert>
#include <stdexcept>


namespace papilio
{
    namespace detail
    {
        void small_vector_impl_base::throw_out_of_range()
        {
            throw std::out_of_range("small vector index out of range");
        }
        void small_vector_impl_base::throw_length_error()
        {
            throw std::length_error("length error");
        }

        void fixed_vector_impl_base::throw_out_of_range()
        {
            throw std::out_of_range("out of range");
        }
        void fixed_vector_impl_base::throw_length_error()
        {
            throw std::length_error("length error");
        }

        void fixed_flat_map_impl_base::throw_out_of_range()
        {
            throw std::out_of_range("out of range");
        }
    }
}
