// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <string>
#include <memory>

class Parser {
    public:
        // Constructor
        explicit Parser(std::vector<Token> tokens);

        // Read first token and decides what to parser
        // Returns AST root node of the query
        ASTNodePointer parse();

    private:
        std::vector<Token> tokens_;
        size_t position_;

        Token& peek();
        Token& peekNext();
        Token& advance();

        bool isAtEnd() const;
        bool check(TokenType type) const;   // Check type token
        bool match(TokenType type); // If check() advance and return True

        // Actual token must be the indicate type
        Token& consume(TokenType type, const std::string& errorMessage);

        // Sentence parsers
        ASTNodePointer parseCreateTable();
        ASTNodePointer parseInsert();
        ASTNodePointer parseSelect();
        ASTNodePointer parseDelete();
        ASTNodePointer parseUpdate();

        // Expressions parsers
        ASTNodePointer parseExpression();
        ASTNodePointer parseAnd();
        ASTNodePointer parseComparison();
        ASTNodePointer parsePrimary();

        ASTNodePointer parseCreateIndex();
};

#endif //PARSER_H