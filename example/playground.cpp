#include <iostream>
#include <functional>
#include <papilio/papilio.hpp>
#include <papilio/util/all.hpp>


namespace playground
{
    class option
    {
    public:
        option() = delete;
        explicit option(std::string name_, char shortcut_ = '\0')
            : m_name(std::move(name_)), m_shortcut(shortcut_) {}

        [[nodiscard]]
        bool has_shortcut() const noexcept
        {
            return m_shortcut != '\0';
        }

        [[nodiscard]]
        std::string to_string() const
        {
            std::string result = m_name;
            if(has_shortcut())
            {
                result += ", ";
                result += m_shortcut;
            }

            return result;
        }

        [[nodiscard]]
        bool match(std::string_view str) const noexcept
        {
            if(str == m_name)
                return true;
            else if(has_shortcut())
                return str.length() == 1 && str[0] == m_shortcut;
            else
                return false;
        }
        bool try_invoke(std::string_view str) const
        {
            if(match(str))
            {
                if(m_callback)
                    m_callback();
                return true;
            }

            return false;
        }

        [[nodiscard]]
        const std::string& name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        const std::string& help() const noexcept
        {
            return m_help;
        }
        void help(std::string msg) noexcept
        {
            m_help = std::move(msg);
        }

        template <typename Function>
        void connect(Function&& func)
        {
            m_callback = std::forward<Function>(func);
        }

    private:
        std::string m_name;
        char m_shortcut;
        std::string m_help;

        std::function<void()> m_callback;
    };

    class interactive_interface
    {
    public:
        using argument_type = std::variant<
            std::int64_t,
            long double,
            std::string
        >;

        interactive_interface()
        {
            build_options();
        }

        std::span<const option> get_options() const
        {
            return m_opts;
        }

        std::string get_help() const
        {
            std::stringstream ss;
            ss << "Help" << std::endl;

            for(auto& opt : m_opts)
            {
                papilio::print(ss, "{:<8} : {}\n", opt.to_string(), opt.help());
            }

            return std::move(ss).str();
        }

        bool try_invoke(std::string_view in) const
        {
            for(const auto& opt : get_options())
            {
                if(opt.try_invoke(in))
                    return true;
            }

            return false;
        }

    private:
        std::vector<option> m_opts;
        std::vector<argument_type> m_args;
        std::string m_fmt;

        template <typename Function>
        void emplace_option(std::string name, char shortcut, std::string help, Function&& func)
        {
            auto& opt = m_opts.emplace_back(std::move(name), shortcut);
            opt.help(std::move(help));
            opt.connect(std::move(func));
        }

        void build_options()
        {
            emplace_option(
                "help", 'h',
                "This message",
                [this]() { papilio::println(get_help()); }
            );
            emplace_option(
                "add", 'a',
                "Add a new argument",
                std::bind(&interactive_interface::add_argument, this)
            );
            emplace_option(
                "print", '\0',
                "Print value of an argument",
                std::bind(&interactive_interface::print_arguement, this)
            );
            emplace_option(
                "list", 'l',
                "List all arguments",
                std::bind(&interactive_interface::list_arguement, this)
            );
            emplace_option(
                "format", 'f',
                "Set format string",
                std::bind(&interactive_interface::set_format_str, this)
            );
            emplace_option(
                "execute", 'e',
                "Execute",
                std::bind(&interactive_interface::execute_fmt, this)
            );
            emplace_option(
                "quit", 'q',
                "Quit",
                std::bind(std::exit, EXIT_SUCCESS)
            );
        }

        void add_argument()
        {
            papilio::print("Select type: string(s), integer(i) or float(f)");
            char ty = '\0';
            std::cin >> ty;
            switch(ty)
            {
            case 's':
            {
                std::string buf;
                std::cin >> buf;
                m_args.emplace_back(std::move(buf));
                break;
            }

            case 'i':
            {
                std::int64_t val;
                std::cin >> val;
                m_args.emplace_back(val);
                break;
            }

            case 'f':
            {
                long double val;
                std::cin >> val;
                m_args.emplace_back(val);
                break;
            }

            default:
                papilio::println("Invalid type specifier '{}'", ty);
                break;
            }
        }
        void print_arguement()
        {
            papilio::print("Input index: ");
            std::size_t idx = -1;
            std::cin >> idx;

            if(idx >= m_args.size())
            {
                papilio::println("Index out of range");
                return;
            }

            auto visitor = [](auto&& v) { papilio::println("{}", v); };
            papilio::print("{} : ", idx);
            std::visit(visitor, m_args[idx]);
        }
        void list_arguement()
        {
            for(std::size_t i = 0; i < m_args.size(); ++i)
            {
                auto visitor = [i](auto&& v) { papilio::println("{:02d} : {}", i, v); };
                std::visit(visitor, m_args[i]);
            }
        }

        void set_format_str()
        {
            papilio::println("Current format string \"{}\"", m_fmt);

            std::string fmt;
            papilio::print("Input new format string: ");
            std::getline(std::cin, fmt, '\n');

            m_fmt = std::move(fmt);
        }

        void execute_fmt()
        {
            papilio::println("Formatting result:");

            papilio::mutable_format_arg_store arg_store;
            for(auto& i : m_args)
            {
                auto visitor = [&](auto&& v)
                {
                    arg_store.push(v);
                };
                std::visit(visitor, i);
            }

            try
            {
                papilio::vprintln_conv(m_fmt, arg_store);
            }
            catch(const std::exception& e)
            {
                papilio::println("Error ({}): {}", typeid(e).name(), e.what());
            }
        }
    };
}

int main()
{
    using namespace papilio;

    papilio::println("Interactive Playground");
    papilio::println(
        "Papilio Charontis v{}",
        join(get_version(), ".")
    );
    println();

    playground::interactive_interface interface;
    std::string in;

    while(true)
    {
        print("Select mode (\"h\" or \"help\" for help): ");
        std::cin >> in;
        std::cin.get();

        if(!std::cin)
            break;

        if(!interface.try_invoke(in))
        {
            papilio::println("Invalid mode: {}", in);
        }
    }

    return 0;
}
