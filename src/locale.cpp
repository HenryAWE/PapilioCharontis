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

char index_grouping(const std::string& grouping, std::size_t idx)
{
    if(grouping.empty()) [[unlikely]]
        return '\0';

    if(idx >= grouping.size())
        return grouping.back();
    else
        return grouping[idx];
}
} // namespace papilio

#include <papilio/detail/suffix.hpp>
