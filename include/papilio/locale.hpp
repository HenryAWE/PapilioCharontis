/**
 * @file locale.hpp
 * @author HenryAWE
 * @brief Locale support and utilities
 */

#ifndef PAPILIO_LOCALE_HPP
#define PAPILIO_LOCALE_HPP

#pragma once

#include <locale>
#include "fmtfwd.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
/**
 * @brief Reference to a locale object.
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

    /**
     * @brief Check if the reference is empty.
     */
    [[nodiscard]]
    bool empty() const noexcept
    {
        return m_loc == nullptr;
    }

    /**
     * @brief Get the referenced locale object.
     *
     * If the reference is empty, `std::locale::classic()` will be returned.
     *
     * @return std::locale The locale object.
     */
    [[nodiscard]]
    std::locale get() const;

    /**
     * @brief A shortcut for `get()`.
     *
     * @return std::locale The locale object.
     */
    operator std::locale() const
    {
        return get();
    }

private:
    const std::locale* m_loc = nullptr;
};

char index_grouping(const std::string& grouping, std::size_t idx);
} // namespace papilio

#include "detail/suffix.hpp"

#endif
