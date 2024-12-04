#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>

int main()
{
    using namespace papilio;

    papilio::println("Papilio Charontis version {0[0]}.{0[1]}.{0[2]}", get_version());
    papilio::println();

    papilio::println("Is terminal: {}", os::is_terminal(stdout));

    {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);

        static_assert(formattable<std::tm>);
        papilio::println("Local time: {:=^32%c}", *tm);
    }

    papilio::println("Library and compiler information:");
    papilio::println("PAPILIO_CPLUSPLUS = {:d}L", PAPILIO_CPLUSPLUS);

#ifdef PAPILIO_COMPILER_MSVC
    papilio::println("PAPILIO_COMPILER_MSVC = {}", PAPILIO_COMPILER_MSVC);
#endif
#ifdef PAPILIO_COMPILER_GCC
    papilio::println("PAPILIO_COMPILER_GCC = {}", PAPILIO_COMPILER_GCC);
#endif
#ifdef PAPILIO_COMPILER_CLANG
    papilio::println("PAPILIO_COMPILER_CLANG = {}", PAPILIO_COMPILER_CLANG);
#endif
#ifdef PAPILIO_COMPILER_CLANG_CL
    papilio::println("PAPILIO_COMPILER_CLANG_CL defined");
#endif

#ifdef PAPILIO_STDLIB_LIBSTDCPP
    papilio::println("PAPILIO_STDLIB_LIBSTDCPP = {}", PAPILIO_STDLIB_LIBSTDCPP);
#endif
#ifdef PAPILIO_STDLIB_LIBCPP
    papilio::println("PAPILIO_STDLIB_LIBCPP = {}", PAPILIO_STDLIB_LIBCPP);
#endif
#ifdef PAPILIO_STDLIB_MSVC_STL
    papilio::println("PAPILIO_STDLIB_MSVC_STL = {}", PAPILIO_STDLIB_MSVC_STL);
#endif

#ifdef PAPILIO_HAS_ATTR_NO_UNIQUE_ADDRESS
    papilio::println("PAPILIO_HAS_ATTR_NO_UNIQUE_ADDRESS = {}", PAPILIO_HAS_ATTR_NO_UNIQUE_ADDRESS);
#endif

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT
    papilio::println("PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT = {:d}L", PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT);
#endif
#ifdef PAPILIO_HAS_LIB_STACKTRACE
    papilio::println("PAPILIO_HAS_LIB_STACKTRACE = {:d}L", PAPILIO_HAS_LIB_STACKTRACE);
#endif
#ifdef PAPILIO_HAS_LIB_EXPECTED
    papilio::println("PAPILIO_HAS_LIB_EXPECTED = {:d}L", PAPILIO_HAS_LIB_EXPECTED);
#endif

    papilio::println("PAPILIO_HAS_UNREACHABLE = {}", PAPILIO_HAS_UNREACHABLE);
#ifdef PAPILIO_HAS_ENUM_NAME
    papilio::println("PAPILIO_HAS_ENUM_NAME = {}", PAPILIO_HAS_ENUM_NAME);
#endif

    return 0;
}
