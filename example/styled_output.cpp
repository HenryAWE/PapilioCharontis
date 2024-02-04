#include <papilio/papilio.hpp>

int main()
{
    using namespace papilio;

    println("Example: Styled Output");
    println("NOTE: The actual output depends on your terminal!");
    println();

    print("Different styles: ");
    print(style::bold, "bold");
    print(", ");
    print(style::italic, "italic");
    print(", ");
    println(style::underline, "underlined");

    println(
        "Colorful output: {}, {}, {}",
        styled(fg(color::red), "red"_sc),
        styled(fg(color::green), "green"_sc),
        styled(fg(color::blue), "blue"_sc)
    );

    println("Combined:");
    println(fg(color::yellow) | bg(color::white) | style::bold, "WARNING");
    println(fg(color::red) | bg(color::white) | style::bold, "ERROR");

    return 0;
}
