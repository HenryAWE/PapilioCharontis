#include <papilio/utf/common.hpp>


namespace papilio::utf
{
    invalid_byte::invalid_byte(std::uint8_t ch)
        : invalid_argument("invalid byte"), m_byte(ch) {}

    invalid_surrogate::invalid_surrogate(std::uint16_t ch)
        : invalid_argument("invalid surrogate"), m_ch(ch) {}
}
