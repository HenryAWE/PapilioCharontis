macro(define_papilio_example example_name)
    add_executable(${example_name} ${example_name}.cpp)
    target_link_libraries(${example_name} PRIVATE papilio)
    # Force UTF-8 on MSVC
    target_compile_options(${example_name} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )
endmacro()

define_papilio_example(info)

define_papilio_example(script_demo)

define_papilio_example(custom_formatter)

define_papilio_example(styled_output)

define_papilio_example(magic_enum)

if(${papilio_build_module})
    define_papilio_example(import)
endif()

add_subdirectory(playground)
