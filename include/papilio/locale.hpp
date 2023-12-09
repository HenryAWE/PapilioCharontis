#pragma once

#include <locale>

namespace papilio
{
// Reference to a locale object
// The member function "get()" will return the C locale if the reference is empty.
class locale_ref
{
public:
    locale_ref() noexcept = default;
    locale_ref(const locale_ref&) noexcept = default;

    locale_ref(const std::locale& loc) noexcept
        : m_loc(&loc) {}

    locale_ref(std::nullptr_t) noexcept
        : m_loc(nullptr) {}

    locale_ref& operator=(const locale_ref&) noexcept = default;

    locale_ref& operator=(const std::locale& loc) noexcept
    {
        m_loc = &loc;
        return *this;
    }

    locale_ref& operator=(std::nullptr_t) noexcept
    {
        m_loc = nullptr;
        return *this;
    }

    [[nodiscard]]
    std::locale get() const
    {
        return m_loc == nullptr ?
                   std::locale("C") :
                   *m_loc;
    }

    operator std::locale() const
    {
        return get();
    }

private:
    const std::locale* m_loc = nullptr;
};
} // namespace papilio
