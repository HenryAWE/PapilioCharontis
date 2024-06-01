#ifndef PAPILIO_LOCALE_HPP
#define PAPILIO_LOCALE_HPP

#pragma once

#include <locale>
#include "macros.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
/**
 * @brief Reference to a locale object.
 *
 * The member function `get()` will return `std::locale::classic()` if the reference is empty.
 */
PAPILIO_EXPORT class locale_ref
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
    bool empty() const noexcept
    {
        return m_loc == nullptr;
    }

    [[nodiscard]]
    std::locale get() const;

    operator std::locale() const
    {
        return get();
    }

private:
    const std::locale* m_loc = nullptr;
};
} // namespace papilio

#include "detail/suffix.hpp"

#endif
