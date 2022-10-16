cmake_minimum_required(VERSION 3.12)

project(qkeycode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The includer is responsible for supplying a Qt target.

add_library(qkeycode STATIC
    src/qkeycode/qkeycode.cpp
    src/qkeycode/chromium/keycode_converter.cc
)
target_include_directories(qkeycode
    PUBLIC include
    PRIVATE src
)
target_link_libraries(qkeycode PRIVATE Qt::Gui)

target_compile_definitions(qkeycode PRIVATE KEYCODEQT_LIBRARY)
