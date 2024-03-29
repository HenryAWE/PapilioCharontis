#include <papilio/color.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
text_style fg(color col) noexcept
{
    text_style st;
    st.m_fg = col;
    return st;
}

text_style bg(color col) noexcept
{
    text_style st;
    st.m_bg = col;
    return st;
}
} // namespace papilio

#include <papilio/detail/suffix.hpp>
