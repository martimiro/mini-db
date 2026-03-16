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
        src/storage/record.cpp
        src/storage/disk_manager.cpp
        src/storage/buffer_pool.cpp
        src/storage/heap_file.cpp
        src/storage/table_manager.cpp
        include/executor/executor.h
        src/executor/executor.cpp
        include/index/btree.h
        src/index/btree.cpp
        include/index/index_manager.h
        src/index/index_manager.cpp
        include/transaction/wal.h
        include/transaction/transaction_manager.h
        src/transaction/wall.cpp
        src/transaction/transaction_manager.cpp
        include/storage/catalog.h
        src/storage/catalog.cpp
)

add_executable(test_lexer
        test/test_lexer.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
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
)

add_executable(test_buffer_pool
        test/test_buffer_pool.cpp
        src/storage/page.cpp
        src/storage/disk_manager.cpp
        src/storage/buffer_pool.cpp
        test/test_executor.cpp
        test/test_btree.cpp
)

add_executable(test_heap_file
        test/test_heap_file.cpp
        src/storage/page.cpp
        src/storage/record.cpp
        src/storage/disk_manager.cpp
        src/storage/buffer_pool.cpp
        src/storage/heap_file.cpp
)

add_executable(test_executor
        test/test_executor.cpp
        src/parser/token.cpp
        src/parser/lexer.cpp
        src/parser/parser.cpp
        src/parser/visitor.cpp
        src/storage/page.cpp
        src/storage/record.cpp
        src/storage/disk_manager.cpp
        src/storage/buffer_pool.cpp
        src/storage/heap_file.cpp
        src/storage/table_manager.cpp
        src/executor/executor.cpp
)

add_executable(test_btree
    test/test_btree.cpp
    src/storage/record.cpp
    src/index/btree.cpp
)
