cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

project(papilio_test_install LANGUAGES CXX)

find_package(Papilio REQUIRED)

add_executable(papilio_test_install "test_install.cpp")
target_link_libraries(papilio_test_install PRIVATE papilio::papilio)
