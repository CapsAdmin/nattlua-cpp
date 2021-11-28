cmake_minimum_required(VERSION 3.5)

project(Nattlua 
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_executable(nattlua
    src/code/Code.cpp
    src/syntax/BaseSyntax.cpp
    src/lexer/BaseLexer.cpp
    src/lexer/LuaLexer.cpp

    src/lexer/LexerTest.cpp
    src/main.cpp
)
