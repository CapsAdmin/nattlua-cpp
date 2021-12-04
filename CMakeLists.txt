cmake_minimum_required(VERSION 3.5)

set(LIBRARY_CPP 
    src/code/Code.cpp
    src/syntax/BaseSyntax.cpp
    src/lexer/BaseLexer.cpp
    src/lexer/LuaLexer.cpp
    src/parser/LuaParser.cpp
    src/parser/PrimaryExpression.cpp
)

set(TEST_CPP 
    tests/Lexer.cpp
    tests/Parser.cpp
)

set(TEST_ARGS
    "-O0"
    "-fprofile-arcs"
    "-ftest-coverage"
)

project(main LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)

    add_executable(main src/main.cpp)
    
    add_library(lib ${LIBRARY_CPP})
    target_link_libraries(main lib)

    #google test
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
        )
        FetchContent_MakeAvailable(googletest)

        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        enable_testing()

        add_executable(tests ${TEST_CPP} ${LIBRARY_CPP})
        target_link_libraries(tests gtest_main gcov)
        target_compile_options(tests PUBLIC ${TEST_ARGS})

        include(GoogleTest)
        gtest_discover_tests(tests)
    #google test