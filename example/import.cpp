#include <string>
import papilio;

int main()
{
    papilio::println("Hello world from imported module!");

    papilio::print(
        papilio::style::italic | papilio::fg(papilio::color::blue) | papilio::bg(papilio::color::white),
        "Styled output"
    );
    papilio::print("\t(italic font + blue foreground + white background)");

    return 0;
}
