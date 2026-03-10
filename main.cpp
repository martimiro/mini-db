#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/visitor.h"
#include "semantic/semantic.h"
#include <iostream>
#include <string>

// REPL -> Read Eval Print Loop
int main() {
    Catalog catalog;
    PrintVisitor printer;
    SemanticAnalyzer analyzer(catalog);

    std::cout << "mini-db v0.1 — type 'exit' to quit\n";

    std::string line;
    while (true) {
        std::cout << "\nmini-db> ";

        // Read line
        if (!std::getline(std::cin, line)) {
            break;
        }

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        // Exit commands
        if (line == "exit" || line == "quit" || line == "\\q") {
            std::cout << "Bye!\n";
            break;
        }

        // Skip empty lines and comments
        if (line.empty() || (line.size() >= 2 && line[0] == '-' && line[1] == '-')) {
            continue;
        }

        // Execute
        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenizeAll();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();

            ast->accept(printer);
            ast->accept(analyzer);
        } catch (const SemanticError& e) {
            std::cout << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << "Parse error: " << e.what() << "\n";
        }
    }

    return 0;
}