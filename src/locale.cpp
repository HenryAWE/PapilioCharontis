#include <papilio/locale.hpp>


namespace papilio
{
    std::locale locale_ref::get() const
    {
        return m_loc == nullptr ?
            std::locale("C") :
            *m_loc;
    }
}
