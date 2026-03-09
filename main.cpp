#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/visitor.h"
#include "semantic/semantic.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<std::string> queries = {
        "CREATE TABLE usuarios (id INT, nombre TEXT, edad INT);",
        "INSERT INTO usuarios VALUES (1, \"Ana\", 25);",
        "SELECT nombre, edad FROM usuarios WHERE edad > 20;",
        "SELECT ciudad FROM usuarios WHERE edad > 20;",   // ← columna inválida
        "DELETE FROM usuarios WHERE id = 1;",
        "UPDATE usuarios SET nombre = \"Luis\" WHERE id = 1;",
        "SELECT * FROM facturas;",                         // ← tabla inválida
    };

    Catalog catalog;
    PrintVisitor printer;
    SemanticAnalyzer analyzer(catalog);

    for (const auto& sql : queries) {
        std::cout << "SQL: " << sql << "\n";
        std::cout << std::string(50, '-') << "\n";
        try {
            Lexer lexer(sql);
            auto tokens = lexer.tokenizeAll();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();

            ast->accept(printer);    // imprimir AST
            ast->accept(analyzer);   // validar semántica
        } catch (const SemanticError& e) {
            std::cout << "❌ " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << "\n";
        }
        std::cout << "\n";
    }

    return 0;
}