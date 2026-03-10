cmake_minimum_required(VERSION 3.20)
project(mini-db)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(mini-db
        main.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        src/parser/parser.cpp
        include/parser/visitor.h
        src/parser/vistor.cpp
        include/semantic/semantic.h
        src/semantic/semantic.cpp
        include/storage/page.h
        src/storage/page.cpp
)

add_executable(test_lexer
        test/test_lexer.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        test/test_parser.cpp
)

add_executable(test_parser
        test/test_parser.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        src/parser/parser.cpp
)