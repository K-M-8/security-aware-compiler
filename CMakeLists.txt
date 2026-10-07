cmake_minimum_required(VERSION 3.16)
project(SecurityAwareCompiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(security_compiler
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/security.cpp
    src/tac.cpp
    src/transform.cpp
    src/codegen.cpp
)
target_include_directories(security_compiler PRIVATE src)
