cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE PATH "c++" FORCE)

project(Nattlua 
    VERSION 0.0.1
    LANGUAGES CXX
)

add_compile_options(-std=c++2a)

file(GLOB_RECURSE sources SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")
add_executable(nattlua ${sources})
