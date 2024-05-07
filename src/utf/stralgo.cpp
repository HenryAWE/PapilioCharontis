#include <papilio/utf/stralgo.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio::utf
{
invalid_byte::~invalid_byte() = default;

invalid_surrogate::~invalid_surrogate() = default;
} // namespace papilio::utf

#include <papilio/detail/suffix.hpp>
