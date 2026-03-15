#include "parser/lexer.h"
#include "parser/parser.h"
#include "executor/executor.h"
#include "storage/table_manager.h"
#include <iostream>
#include <string>

int main() {
    TableManager tableManager("./data");
    Executor executor(tableManager);

    std::cout << "mini-db v0.1 — type 'exit' to quit\n";

    std::string line;
    while (true) {
        std::cout << "\nmini-db> ";

        if (!std::getline(std::cin, line)) break;

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        if (line == "exit" || line == "quit" || line == "\\q") {
            std::cout << "Bye!\n";
            break;
        }

        if (line.empty() || (line.size() >= 2 && line[0] == '-' && line[1] == '-')) {
            continue;
        }

        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenizeAll();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();
            ast->accept(executor);
        } catch (const std::exception& e) {
            std::cout  << e.what() << "\n";
        }
    }

    return 0;
}