#include <papilio/print.hpp>
#include <papilio/os/os.hpp>
#include <papilio/detail/prefix.hpp>

namespace papilio
{
namespace detail
{
    static void output_impl(
        std::FILE* file,
        std::string_view out,
        bool conv_unicode
    )
    {
        if(conv_unicode)
            os::output_conv(file, out);
        else
            os::output_nonconv(file, out);
    }

    void vprint_impl(
        std::FILE* file,
        std::string_view fmt,
        format_args_ref args,
        bool conv_unicode,
        bool newline,
        text_style st
    )
    {
        std::string out;
        {
            auto it = std::back_inserter(out);

            st.set(it);
            out += PAPILIO_NS vformat(fmt, args);
            st.reset(it);
        }
        if(newline)
            out.push_back('\n');

        output_impl(file, out, conv_unicode);
    }
} // namespace detail

void println(std::FILE* file)
{
    detail::output_impl(file, "\n", os::is_terminal(file));
}

void println()
{
    println(stdout);
}

void println(std::ostream& os)
{
    os << '\n';
}
} // namespace papilio

#include <papilio/detail/suffix.hpp>
