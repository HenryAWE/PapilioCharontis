#include <iostream>
#include <papilio/papilio.hpp>

/*
1. Explicitly specialize a "papilio::formatter" template
*/

namespace custom_formatter
{
class use_template_spec
{
public:
    use_template_spec(char ch, int count)
        : m_count(count), m_ch(ch) {}

    char get_ch() const
    {
        return m_ch;
    }

    int get_count() const
    {
        return m_count;
    }

private:
    int m_count;
    char m_ch;
};
} // namespace custom_formatter

namespace papilio
{
template <>
class formatter<custom_formatter::use_template_spec>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        auto it = ctx.begin();

        if(*it == U's')
        {
            m_as_str = true;
            ++it;
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const custom_formatter::use_template_spec& val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        if(m_as_str)
        {
            using context_t = format_context_traits<FormatContext>;
            context_t::append(ctx, val.get_ch(), val.get_count());

            return ctx.out();
        }
        else
        {
            // Use "papilio::" prefix to avoid ADL
            return papilio::format_to(
                ctx.out(), "({}, {})", val.get_ch(), val.get_count()
            );
        }
    }

private:
    bool m_as_str = false;
};
} // namespace papilio

/*
2. Generate formatter by the overloaded "operator<<"
*/

namespace custom_formatter
{
class use_ostream
{
public:
    use_ostream(int val)
        : m_val(val) {}

    friend std::ostream& operator<<(std::ostream& os, const use_ostream& val)
    {
        os << "Use ostream: " << val.m_val;
        return os;
    }

private:
    int m_val;
};
} // namespace custom_formatter

int main()
{
    using namespace custom_formatter;

    papilio::println("Example: Custom Formatter");
    papilio::println();

    papilio::println("1. Template Specialization");
    {
        use_template_spec val('A', 3);

        static_assert(papilio::formattable<use_template_spec>);

        papilio::println("Fmt={{}}\n{}", val);
        papilio::println("Fmt={{:s}}\n{:s}", val);
    }
    papilio::println();

    papilio::println("2. std::ostream Compatibility");
    {
        use_ostream val(42);

        static_assert(papilio::formattable<use_ostream>);

        papilio::println("{}", val);
    }
    return 0;
}
