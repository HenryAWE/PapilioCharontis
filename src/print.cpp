#include <papilio/print.hpp>
#include <system_error>


namespace papilio
{
    // output iterator for C FILE*
    class cfile_iterator
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        cfile_iterator() = delete;
        cfile_iterator(const cfile_iterator&) noexcept = default;
        cfile_iterator(std::FILE* file) noexcept
            : m_file(file) {}

        cfile_iterator& operator=(const cfile_iterator&) noexcept = default;
        cfile_iterator& operator=(char ch)
        {
            int result = std::fputc(ch, m_file);
            if(result == EOF)
            {
                throw std::system_error(
                    std::make_error_code(std::errc::io_error)
                );
            }
            return *this;
        }

        cfile_iterator& operator*() noexcept
        {
            return *this;
        }
        cfile_iterator& operator++() noexcept { return *this; }
        cfile_iterator operator++(int) noexcept { return *this; }

    private:
        std::FILE* m_file;
    };

    void vprint(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store)
    {
        vformat_to(cfile_iterator(file), fmt, store);
    }
    void vprintln(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store)
    {
        auto it = vformat_to(cfile_iterator(file), fmt, store);
        *it = '\n';
    }

    void println(std::FILE* file)
    {
        cfile_iterator it(file);
        *it = '\n';
    }
    void println()
    {
        println(stdout);
    }
}
