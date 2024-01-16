#pragma once

#include <span>
#include <map>
#include <papilio/papilio.hpp>

// Interactive console
class ipapilio
{
public:
    ipapilio();

    ipapilio(const ipapilio&) = delete;

    ipapilio& operator=(const ipapilio&) = delete;

    void mainloop();

private:
    bool m_quit = false;

    using cmd_args_t = std::span<const std::string>;

    struct command_data
    {
        using callback_t = void (ipapilio::*)();

        std::string help;
        callback_t callback;
    };

    std::map<std::string, command_data, std::less<>> m_cmds;

    std::string m_fmt;
    papilio::mutable_format_args m_args;

    void build_cmd();

    void quit();
    void print_help();

    void addi();
    void addf();
    void adds();

    void setf();

    void list_arg();

    void clear_arg();

    void run();
};
