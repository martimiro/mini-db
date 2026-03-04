cmake_minimum_required(VERSION 3.20)
project(mini-db)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(mini-db
        src/main.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
)

add_executable(test_lexer
        test/test_lexer.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
)