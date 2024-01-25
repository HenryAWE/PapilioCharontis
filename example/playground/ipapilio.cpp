#include "ipapilio.hpp"
#include <iostream>
#include <charconv>
#include <vector>
#include <papilio/print.hpp>

namespace detail
{
static std::string input(std::istream& is, std::string_view prefix = "")
{
    if(prefix.empty())
        papilio::print(papilio::style::faint, "> ");
    else
        papilio::print(papilio::style::faint, "[{}]> ", prefix);

    std::string result;
    is >> std::ws;
    std::getline(is, result);

    while(!result.empty() && papilio::utf::is_whitespace(result.back()))
        result.pop_back();

    return result;
}
} // namespace detail

ipapilio::ipapilio()
{
    build_cmd();
}

void ipapilio::mainloop()
{
    while(!m_quit)
    {
        std::string cmd_id = detail::input(std::cin);
        if(std::cin.eof())
        {
            papilio::println("EOF received");
            break;
        }
        else if(!std::cin)
        {
            papilio::println("Input error");
            break;
        }

        if(cmd_id.empty())
            continue;
        auto it = m_cmds.find(cmd_id);
        if(it != m_cmds.end())
        {
            (this->*it->second.callback)();
        }
        else
        {
            papilio::println("Invalid command: {}", cmd_id);
        }
    }
}

void ipapilio::build_cmd()
{
    auto emplace_cmd = [this](
                           std::string name,
                           std::string help_msg,
                           command_data::callback_t cb
                       )
    {
        m_cmds.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(std::move(name)),
            std::forward_as_tuple(std::move(help_msg), cb)
        );
    };

    emplace_cmd("quit", "Quit", &ipapilio::quit);
    emplace_cmd("help", "This message", &ipapilio::print_help);

    emplace_cmd("addi", "Add an integer argument", &ipapilio::addi);
    emplace_cmd("addf", "Add a floating point argument", &ipapilio::addf);
    emplace_cmd("adds", "Add a string argument", &ipapilio::adds);

    emplace_cmd("setf", "Set the format string", &ipapilio::setf);

    emplace_cmd("clear", "Clear all arguments", &ipapilio::clear_arg);

    emplace_cmd("ls", "List arguments and format string", &ipapilio::list_arg);

    emplace_cmd("run", "Ouput format result", &ipapilio::run);
}

void ipapilio::quit()
{
    papilio::println("Quit");
    m_quit = true;
}

void ipapilio::print_help()
{
    papilio::println("Help");
    for(const auto& [key, cmd] : m_cmds)
    {
        papilio::println("{:<6} : {}", key, cmd.help);
    }
}

void ipapilio::addi()
{
    std::string str_val = detail::input(std::cin, "addi");

    std::int64_t val;
    auto result = std::from_chars(
        std::to_address(str_val.begin()),
        std::to_address(str_val.end()),
        val
    );
    if(result.ec != std::errc())
    {
        papilio::println(
            papilio::fg(papilio::color::red) | papilio::style::bold,
            "Bad value: {}",
            std::make_error_code(result.ec).message()
        );
        return;
    }

    m_args.push(val);
    papilio::println(
        "Added integer argument: {}",
        papilio::styled(papilio::fg(papilio::color::green), val)
    );
}

void ipapilio::addf()
{
    std::string str_val = detail::input(std::cin, "addf");

    long double val;
    auto result = std::from_chars(
        std::to_address(str_val.begin()),
        std::to_address(str_val.end()),
        val
    );
    if(result.ec != std::errc())
    {
        papilio::println(
            papilio::fg(papilio::color::red) | papilio::style::bold,
            "Bad value: {}",
            std::make_error_code(result.ec).message()
        );
        return;
    }

    m_args.push(val);
    papilio::println(
        "Added floating point argument: {}",
        papilio::styled(papilio::fg(papilio::color::green), val)
    );
}

void ipapilio::adds()
{
    papilio::utf::string_container str = detail::input(std::cin, "adds");
    PAPILIO_ASSERT(str.has_ownership());

    papilio::println(
        "Added string argument: {}",
        papilio::styled(papilio::fg(papilio::color::yellow), str)
    );
    m_args.push(std::move(str));
}

void ipapilio::setf()
{
    m_fmt = detail::input(std::cin, "setf");
    papilio::println(
        "Set format string: {}",
        papilio::styled(papilio::fg(papilio::color::yellow), papilio::utf::string_container(m_fmt))
    );
}

void ipapilio::list_arg()
{
    papilio::println(
        "Format string: {}\n"
        "Arguments: {} indexed, {} named",
        m_fmt,
        m_args.indexed_size(),
        m_args.named_size()
    );
}

void ipapilio::clear_arg()
{
    m_args.clear();
    papilio::println("Cleared");
}

void ipapilio::run()
{
    using namespace papilio;
    using intp_t = script::basic_interpreter<format_context, true>;

    utf::string_ref fmt = m_fmt;
    std::string result;

    try
    {
        intp_t intp;

        format_parse_context parse_ctx(fmt, m_args);
        format_context fmt_ctx(std::back_inserter(result), m_args);

        intp.format(parse_ctx, fmt_ctx);
    }
    catch(const intp_t::extended_error& e)
    {
        utf::string_ref parsed(fmt.begin(), e.get_iter());

        script::script_error_code ec = e.error_code();
        std::size_t pos = parsed.length() + 1;
        papilio::println(fg(color::yellow), "{}", fmt);
        papilio::println(
            "{:~>{}}", styled(fg(color::red) | style::bold, utf::codepoint('^')), pos
        );
        papilio::println(
            "{: >{}} (0x{:X})",
            to_string(ec),
            pos,
            to_underlying(ec)
        );
        return;
    }
    catch(const intp_t::error& e)
    {
        if(e.error_code() == script::script_error_code::end_of_string)
        {
            const script::script_error_code ec = script::script_error_code::end_of_string;
            std::size_t pos = fmt.length() + 1;
            papilio::println(fg(color::yellow), "{}", fmt);
            papilio::println(
                "{:~>{}}", styled(fg(color::red) | style::bold, utf::codepoint('^')), pos
            );
            papilio::println(
                "{: >{}} (0x{:X})",
                to_string(ec),
                pos,
                to_underlying(ec)
            );
        }
        else
        {
            papilio::println(
                "Script error: {} (0x{:X})",
                e.what(),
                static_cast<std::underlying_type_t<script::script_error_code>>(e.error_code())
            );
        }
        return;
    }
    catch(const papilio::format_error& e)
    {
        papilio::println("Format error: {}", e.what());
        return;
    }
    catch(const std::exception& e)
    {
        papilio::println("Error ({.name}): {}", typeid(e), e.what());
        return;
    }
    catch(...)
    {
        papilio::println("Unknown error");
        return;
    }

    papilio::println("Result:");
    papilio::println(fg(color::yellow), "{}", result);
    if(result.empty())
    {
        papilio::println("(Empty string)");
    }
    else if(result.back() != '\n')
    {
        papilio::println("(No newline at the end)");
    }
}
