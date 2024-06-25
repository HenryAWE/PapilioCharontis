# Example
NOTE: `papilio_build_example` needs to be set to `ON` to build the examples. See [Custom Build](custom_build.md) for more information.

## Compilation Information
- In `example/info.cpp`

Outputs library version number, as well as compiler and standard library information.

## Script Demo
- In `example/script_demo.cpp`

Demonstrates how the logic control in the script works in an internationalization (i18n) scenario.

## Styled Output
- In `example/styled_output.cpp`

Outputs a string with custom colors and fonts on a terminal that supports ANSI escape sequences.

## Interactive Playground
- In the `example/playground/` directory.
- The binary executable file is named `ipapilio`, located in the `${CMAKE_BINARY_DIR}/example/` directory.

An interactive program for testing format output. You can set your own format strings, arguments, etc. Type "help" in the program to see available commands.
