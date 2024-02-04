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
#ifdef PAPILIO_COMPILER_CLANG
    println("PAPILIO_COMPILER_CLANG = {}", PAPILIO_COMPILER_CLANG);
#endif

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT
    println("PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT = {:d}L", PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT);
#endif

#ifdef PAPILIO_HAS_VA_OPT
    println("PAPILIO_HAS_VA_OPT = {:d}", PAPILIO_HAS_VA_OPT);
#endif

    return 0;
}
