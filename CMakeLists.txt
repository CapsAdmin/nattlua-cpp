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
    tests/Analyzer.cpp
)

set(TEST_ARGS "-O0")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND TEST_ARGS 
        "-fprofile-arcs"
        "-ftest-coverage"
    )
endif()

project(main LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)

    add_executable(main src/main.cpp)
    
    add_library(lib ${LIBRARY_CPP})
    target_link_libraries(main lib)

    #google test
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/3e0e32ba300ce8afe695ad3ba7e81b21b7cf237a.zip
        )
        FetchContent_MakeAvailable(googletest)

        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        enable_testing()

        add_executable(tests ${TEST_CPP} ${LIBRARY_CPP})
        target_link_libraries(tests gtest_main)
        
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_link_libraries(tests gcov)
        endif ()

        target_compile_options(tests PUBLIC ${TEST_ARGS})

        include(GoogleTest)
        gtest_discover_tests(tests)
    #google test