#include <papilio/papilio.hpp>


int main()
{
    using namespace papilio;

    println("hello world");
    println("{{special}} [[characters]]");
    println();

    println("Format Specifiers:");
    print("character:{}; ", 'A');
    print("integer:{}; ", 1234);
    print("float:{}; ", 1.5f);
    print("string:{}; ", "text");
    print("Boolean:{},{} ", true, false);
    println();
    println();

    std::string_view src =
        "There "
        "[if $0 != 1: 'are' else: 'is']"
        " {} "
        "apple[if $0 != 1: 's']";
    println("Script:");
    println(src, 1);
    println(src, 2);

    return 0;
}
