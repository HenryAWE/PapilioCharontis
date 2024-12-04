macro(define_papilio_example example_name)
    add_executable(${example_name} ${example_name}.cpp)
    target_link_libraries(${example_name} PRIVATE papilio)
    # Force UTF-8 on MSVC
    target_compile_options(${example_name} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )
endmacro()

define_papilio_example(info)
if(WIN32)
    target_compile_definitions(info PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
set_target_properties(info PROPERTIES
    CXX_STANDARD ${papilio_internal_cxx_std}
)

define_papilio_example(script_demo)

define_papilio_example(custom_formatter)

define_papilio_example(styled_output)

define_papilio_example(magic_enum)

if(${papilio_build_module})
    define_papilio_example(import)
    target_compile_options(import PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd5050>)
endif()

add_subdirectory(playground)
