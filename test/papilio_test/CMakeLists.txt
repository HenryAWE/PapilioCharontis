add_library(papilio_test INTERFACE)
target_include_directories(papilio_test INTERFACE include)
target_link_libraries(papilio_test INTERFACE papilio)
target_link_libraries(papilio_test INTERFACE GTest::gtest GTest::gtest_main)
target_compile_options(papilio_test INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    # Workaround for GCC and GoogleTest, see https://github.com/google/googletest/issues/4232
    $<$<CXX_COMPILER_ID:GNU>:-Wno-error=restrict -Wno-restrict>
)
# Force UTF-8 on MSVC
target_compile_options(papilio_test INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)
