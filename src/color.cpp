#include <papilio/color.hpp>

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat"
#endif

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

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif
