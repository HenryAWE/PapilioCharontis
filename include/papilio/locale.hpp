/**
 * @file locale.hpp
 * @author HenryAWE
 * @brief Locale support.
 */

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
     *
     * @return true If the reference is empty.
     * @return false If the reference is not empty.
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
} // namespace papilio

#ifdef PAPILIO_STDLIB_MSVC_STL

// Add char8_t, char16_t, and char32_t specialization for std::numpunct,
// in order to avoid C2491 error.
// See: https://stackoverflow.com/questions/48716223/compile-error-for-char-based-stl-stream-containers-in-visual-studio

#    ifdef PAPILIO_COMPILER_CLANG
#        pragma clang diagnostic push
#        pragma clang diagnostic ignored "-Wglobal-constructors"
#    endif

namespace std
{
template <>
class numpunct<char8_t> : public locale::facet
{
public:
    using char_type = char8_t;
    using string_type = u8string;

    static inline locale::id id;

    explicit numpunct(size_t refs = 0)
        : locale::facet(refs) {}

    char_type decimal_point() const
    {
        return do_decimal_point();
    }

    char_type thousands_sep() const
    {
        return do_thousands_sep();
    }

    string grouping() const
    {
        return do_grouping();
    }

    string_type truename() const
    {
        return do_truename();
    }

    string_type falsename() const
    {
        return do_falsename();
    }

protected:
    ~numpunct()
    {
        // empty
    }

    virtual char_type do_decimal_point() const
    {
        return U'.';
    }

    virtual char_type do_thousands_sep() const
    {
        return U',';
    }

    virtual string do_grouping() const
    {
        return "";
    }

    virtual string_type do_truename() const
    {
        return u8"true";
    }

    virtual string_type do_falsename() const
    {
        return u8"false";
    }
};

template <>
class numpunct<char16_t> : public locale::facet
{
public:
    using char_type = char16_t;
    using string_type = u16string;

    static inline locale::id id;

    explicit numpunct(size_t refs = 0)
        : locale::facet(refs) {}

    char_type decimal_point() const
    {
        return do_decimal_point();
    }

    char_type thousands_sep() const
    {
        return do_thousands_sep();
    }

    string grouping() const
    {
        return do_grouping();
    }

    string_type truename() const
    {
        return do_truename();
    }

    string_type falsename() const
    {
        return do_falsename();
    }

protected:
    ~numpunct()
    {
        // empty
    }

    virtual char_type do_decimal_point() const
    {
        return U'.';
    }

    virtual char_type do_thousands_sep() const
    {
        return U',';
    }

    virtual string do_grouping() const
    {
        return "";
    }

    virtual string_type do_truename() const
    {
        return u"true";
    }

    virtual string_type do_falsename() const
    {
        return u"false";
    }
};

template <>
class numpunct<char32_t> : public locale::facet
{
public:
    using char_type = char32_t;
    using string_type = u32string;

    static inline locale::id id;

    explicit numpunct(size_t refs = 0)
        : locale::facet(refs) {}

    char_type decimal_point() const
    {
        return do_decimal_point();
    }

    char_type thousands_sep() const
    {
        return do_thousands_sep();
    }

    string grouping() const
    {
        return do_grouping();
    }

    string_type truename() const
    {
        return do_truename();
    }

    string_type falsename() const
    {
        return do_falsename();
    }

protected:
    ~numpunct()
    {
        // empty
    }

    virtual char_type do_decimal_point() const
    {
        return U'.';
    }

    virtual char_type do_thousands_sep() const
    {
        return U',';
    }

    virtual string do_grouping() const
    {
        return "";
    }

    virtual string_type do_truename() const
    {
        return U"true";
    }

    virtual string_type do_falsename() const
    {
        return U"false";
    }
};
} // namespace std

#    ifdef PAPILIO_COMPILER_CLANG
#        pragma clang diagnostic pop
#    endif

#endif

#include "detail/suffix.hpp"

#endif
