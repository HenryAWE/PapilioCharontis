#include <papilio/papilio.hpp>

enum my_color
{
    red,
    green,
    blue
};

int main()
{
    papilio::println("{0:d}: {0}", my_color::red);
    papilio::println("{0:d}: {0}", my_color::green);
    papilio::println("{0:d}: {0}", my_color::blue);

    return 0;
}
