#pragma once

#include <utility>
#include "../macros.hpp"


namespace papilio::detail
{
    // Implement unreachable() of C++ 23 for compatibility.
    [[noreturn]]
    inline void unreachable()
    {
#ifdef __cpp_lib_unreachable
        std::unreachable();
#elif defined PAPILIO_COMPILER_MSVC
        __assume(false);
#elif defined PAPILIO_COMPILER_GCC
        __builtin_unreachable();
#else
        // An empty function body and the [[noreturn]] attribute is enough to raise undefined behavior.
#endif
    }
}
