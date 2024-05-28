#include <memory>
#include "ipapilio.hpp"
#ifdef PAPILIO_PLATFORM_WINDOWS
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#endif

std::unique_ptr<ipapilio> interactive_con;

int main(int argc, char* argv[])
{
    interactive_con = std::make_unique<ipapilio>();

    if(!papilio::os::is_terminal(stdout))
    {
        // Not running in a terminal, print information and quit
        interactive_con->print_info();
        return 0;
    }

#ifdef PAPILIO_PLATFORM_WINDOWS
    SetConsoleCP(CP_UTF8);
#endif

    interactive_con->mainloop();

    return 0;
}
