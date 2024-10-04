// NOTE: This header intentionally does not contain a header guard!

#include "config.hpp"

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat"
#    pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#    pragma clang diagnostic ignored "-Wc++98-c++11-compat-binary-literal"
#    pragma clang diagnostic ignored "-Wpre-c++17-compat"
#    pragma clang diagnostic ignored "-Wpre-c++20-compat-pedantic"
#    pragma clang diagnostic ignored "-Wc++20-compat"

#    pragma clang diagnostic ignored "-Wpadded"
#    pragma clang diagnostic ignored "-Wcovered-switch-default"
#    pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#    pragma clang diagnostic ignored "-Wnon-virtual-dtor"

#    if __clang_major__ >= 16
#        pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#    endif
#endif
