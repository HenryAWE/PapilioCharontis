# Custom Formatter
Define a custom type
```c++
class my_value
{
public:
    char ch;
    int count;
};
```
Implement formatter:  
- Ouput `(ch, count)` for empty specification, which is the default behavior
- Output `ch` for `count` times for `s` specification
```c++
namespace papilio
{
    template <>
    class formatter<test_format::my_value>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            std::string_view view(spec.begin(), spec.end());
            if(view == "s")
                m_as_str = true;
        }
        template <typename Context>
        void format(const test_format::my_value& val, Context& ctx)
        {
            format_context_traits traits(ctx);
            if(m_as_str)
                traits.append(val.ch, val.count);
            else
            {
                std::string result;
                result += '(';
                result += val.ch;
                result += ", ";
                result += std::to_string(val.count);
                result += ')';
                traits.append(result);
            }
        }

    private:
        bool m_as_str = false;
    };
}
```
User code
```c++
my_value my_val('B', 3);
format("{}", my_val); // Returns "(B, 3)"
format("{:s}", my_val); // Returns "BBB"
```