cmake_minimum_required(VERSION 3.20)
project(mini-db)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(mini-db
        main.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        src/parser/parser.cpp
        src/parser/visitor.cpp
        src/semantic/semantic.cpp
        src/storage/page.cpp
        include/storage/record.h
        src/storage/record.cpp
        include/storage/disk_manager.h
        src/storage/disk_manager.cpp
        include/storage/buffer_pool.h
        src/storage/buffer_pool.cpp
        include/storage/heap_file.h
        src/storage/heap_file.cpp
)

add_executable(test_lexer
        test/test_lexer.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        test/test_disk_manager.cpp
)

add_executable(test_parser
        test/test_parser.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        src/parser/parser.cpp
        src/parser/visitor.cpp
)

add_executable(test_page
        test/test_page.cpp
        src/storage/page.cpp
)

add_executable(test_disk_manager
        test/test_disk_manager.cpp
        src/storage/page.cpp
        src/storage/disk_manager.cpp
        test/test_buffer_pool.cpp
)

add_executable(test_buffer_pool
        test/test_buffer_pool.cpp
        src/storage/page.cpp
        src/storage/disk_manager.cpp
        src/storage/buffer_pool.cpp
)