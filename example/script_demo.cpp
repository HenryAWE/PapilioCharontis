#include <papilio/papilio.hpp>

void run_script_with(std::string_view fmt, std::initializer_list<int> il)
{
    papilio::println("     fmt: {}", fmt);
    for(int i : il)
    {
        papilio::println(
            "{:>8}: {}",
            papilio::format("{{0}} ={:2}", i),
            papilio::format(fmt, i)
        );
    }
}

void example_1()
{
    papilio::println("English");
    run_script_with("There {${0}!=1?'are':'is'} {0} apple{${0}!=1?'s'}", {1, 2});

    papilio::println("French");
    run_script_with("Il y a {0} pomme{${0}>1?'s'}", {1, 2});

    papilio::println("Chinese");
    run_script_with("有{}个苹果", {1, 2});
}

void example_2()
{
    papilio::println("English");
    run_script_with(
        "There {${0}!=1?'are':'is'} {${0} == 0? 'no' : {0}} apple{${0}!=1?'s'}",
        {0, 1, 2}
    );

    papilio::println("French");
    run_script_with(
        "Il {${0}==0?'n\\''}y a {${0}!=0? {0}: 'pas de'} pomme{${0}>1?'s'}",
        {0, 1, 2}
    );

    papilio::println("Chinese");
    run_script_with(
        "{$!{0}? '没'}有{${0}? {0}}{${0}? '个'}苹果",
        {0, 1, 2}
    );
}

void example_3()
{
    papilio::println("Polish");
    run_script_with(
        "{${0}==1?'plik' : ${0}<5? 'pliki': ${0}<22? 'plików' : ${0}<25? 'pliki' : 'plików'}",
        {1, 3, 5, 24, 31}
    );
}

int main()
{
    papilio::println(papilio::style::bold, "Example: Script Demo");
    papilio::println();

    papilio::println(papilio::style::bold, "Example 1: Simple Script");
    example_1();
    papilio::println();

    papilio::println(papilio::style::bold, "Example 2: Branch with Replacement Field");
    example_2();
    papilio::println();

    papilio::println(papilio::style::bold, "Example 3: Multi-Branch");
    example_3();
    papilio::println();

    return 0;
}
