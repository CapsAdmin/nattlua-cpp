cmake_minimum_required(VERSION 3.5)

project(main LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(
    lib
    src/code/Code.cpp
    src/syntax/BaseSyntax.cpp
    src/lexer/BaseLexer.cpp
    src/lexer/LuaLexer.cpp
)

add_executable(main
    src/main.cpp
)

target_link_libraries(main lib)

#google test
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
    tests
    tests/LexerTest.cpp
)

include(GoogleTest)
gtest_discover_tests(tests)

target_link_libraries(
    tests
    gtest_main
    gcov
    lib
)

target_compile_options(tests
    PUBLIC
        "-O0"
        "-fprofile-arcs"
        "-ftest-coverage"
    )

#google test