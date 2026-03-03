// lexer.h
// Component that generates tokens

#ifndef MINI_DB_LEXER_H
#define MINI_DB_LEXER_H

#include "token.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
    private:
        std::string source_;
        size_t position_;
        int line_;
        int column_;

        // Same structure as a dictionary -> (key, value). Ex: "SELECT" -> TOKEN_KEYWORD_SELECT
        static const std::unordered_map<std::string, TokenType> KEYWORDS;

        char peek() const;          // See actual without advancing
        char peekNext() const;      // See next without advancing
        char advance();             // Read and advance
        char isAtEnd() const;

        void skipWhitespace();
        void skipLineComment();

        // nextToken will call these methods depending on the value of the first
        Token readIndefierOrKeyword();
        Token readNumber();
        Token readString();
        Token readOperatorOrPunctuation();

        std::string toUpperCase(const std::string& str) const;

    public:
        explicit Lexer(const std::string& source);
        Token nextToken();
        std::vector<Token> tokenizeAll();
};

#endif //MINI_DB_LEXER_H