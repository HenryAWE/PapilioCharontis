#include <memory>
#include "ipapilio.hpp"

std::unique_ptr<ipapilio> interactive_con;

int main(int argc, char* argv[])
{
    interactive_con = std::make_unique<ipapilio>();

    interactive_con->mainloop();

    return 0;
}
