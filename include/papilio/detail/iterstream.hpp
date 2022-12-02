#pragma once

#include <iosfwd>
#include <utility>
#include <type_traits>
#include <iterator>


namespace papilio::detail
{
    // helper
    template <bool Enable, typename CharT>
    struct basic_iterbuf_input_buf
    {
        [[nodiscard]]
        CharT* gbuf_ptr() noexcept
        {
            return &m_buf_ch;
        }

    private:
        CharT m_buf_ch = static_cast<CharT>(0);
    };
    template <typename CharT>
    class basic_iterbuf_input_buf<false, CharT> {};

    // output iterator stream buffer
    template <typename CharT, std::input_or_output_iterator Iterator>
    class basic_iterbuf :
        public std::basic_streambuf<CharT>, public basic_iterbuf_input_buf<std::input_iterator<Iterator>, CharT>
    {
        using base = std::basic_streambuf<CharT>;
    public:
        using char_type = CharT;
        using iterator = Iterator;
        using int_type = base::int_type;
        using traits_type = base::traits_type;

        basic_iterbuf() = delete;
        basic_iterbuf(Iterator iter) noexcept(std::is_nothrow_move_constructible_v<Iterator>)
            : basic_iterbuf(std::in_place, std::move(iter)) {}
        template <typename... Args>
        basic_iterbuf(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<Iterator, Args...>)
            : m_iter(std::forward<Args>(args)...) {}

        [[nodiscard]]
        Iterator get() const
        {
            return m_iter;
        }

    protected:
        int_type underflow() override
        {
            if constexpr(std::input_iterator<Iterator>)
            {
                CharT c = *m_iter;
                ++m_iter;

                *this->gbuf_ptr() = c;
                intput_setg();
                return traits_type::to_int_type(c);
            }
            else
            {
                return base::underflow();
            }
        }

        int_type overflow(int_type c) override
        {
            if constexpr(std::output_iterator<Iterator, CharT>)
            {
                *m_iter = c;
                ++m_iter;
                return c;
            }
            else
            {
                return base::overflow(c);
            }
        }

    private:
        Iterator m_iter;

        void intput_setg()
        {
            if constexpr(std::input_iterator<Iterator>)
            {
                CharT* ptr = this->gbuf_ptr();
                this->setg(ptr, ptr, ptr + 1);
            }
        }
    };
}
