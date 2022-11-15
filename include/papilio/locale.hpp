#pragma once

#include <locale>


namespace papilio
{
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

        std::locale get() const;
        operator std::locale() const
        {
            return get();
        }

    private:
        const std::locale* m_loc = nullptr;
    };
}
