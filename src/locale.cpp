#include <papilio/locale.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
std::locale locale_ref::get() const
{
    return m_loc == nullptr ?
               std::locale::classic() :
               *m_loc;
}
} // namespace papilio

#include <papilio/detail/suffix.hpp>
