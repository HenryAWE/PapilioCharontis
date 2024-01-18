#include <papilio/papilio.hpp>

int main()
{
    using namespace papilio;

    println("Papilio Charontis version {0[0]}.{0[1]}.{0[2]}", get_version());

    println();
    println("__cplusplus = {}", __cplusplus);

#ifdef PAPILIO_COMPILER_MSVC
    println("PAPILIO_COMPILER_MSVC = {}", PAPILIO_COMPILER_MSVC);
#endif
#ifdef PAPILIO_COMPILER_GCC
    println("PAPILIO_COMPILER_GCC = {}", PAPILIO_COMPILER_GCC);
#endif

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT
    println("PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT = {:d}L", PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT);
#endif

#ifdef PAPILIO_HAS_VA_OPT
    println("PAPILIO_HAS_VA_OPT = {:d}L", PAPILIO_HAS_VA_OPT);
#endif

    println();

    println("Script test:");

    auto test_script = [](format_string<int> fmt)
    {
        println(fmt, 1);
        println(fmt, 2);

        println();
    };

    // English
    test_script("There {${0}!=1:'are':'is'} {0} big apple{${0}>1:'s'}");
    // Esperanto
    test_script("Estas {0} granda{${0}>1:'j'} pomo{${0}>1:'j'}");

    print(fg(color::red) | bg(color::white) | style::bold, "WARNING");
    println(fg(color::yellow) | style::underline, ", underlined");

    println(
        "{}, {}, {}",
        styled(fg(color::red), "red"_sc),
        styled(fg(color::green), "green"_sc),
        styled(fg(color::blue), "blue"_sc)
    );

    return 0;
}
