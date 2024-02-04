#include <papilio/papilio.hpp>

int main()
{
    using namespace papilio;

    println("Example: Script Demo");
    println();

    auto run_script = [](std::string fmt)
    {
        println("fmt=\"{}\"", fmt);
        println(fmt, 1);
        println(fmt, 2);

        println();
    };

    println("English");
    run_script("There {${0}!=1:'are':'is'} {0} apple{${0}>1:'s'}");

    println("French");
    run_script("Il y a {0} pomme{${0}>1:'s'}");

    println("Chinese");
    run_script("有 {} 个苹果");

    return 0;
}
