// lexer.cpp
#include "../../include/parser/lexer.h"

#include <stdexcept>
#include <algorithm>
#include <cctype>

// Keywords Table
// O(1) complexity thanks to unordered_map
const std::unordered_map<std::string, TokenType> Lexer::KEYWORDS = {
    // Keywords (SQL)
    {"SELECT",              TokenType::TOKEN_KEYWORD_SELECT},
    {"FROM",                TokenType::TOKEN_KEYWORD_FROM},
    {"WHERE",               TokenType::TOKEN_KEYWORD_WHERE},
    {"INSERT",              TokenType::TOKEN_KEYWORD_INSERT},
    {"INTO",                TokenType::TOKEN_KEYWORD_INTO},
    {"VALUES",              TokenType::TOKEN_KEYWORD_VALUES},
    {"CREATE",              TokenType::TOKEN_KEYWORD_CREATE},
    {"TABLE",               TokenType::TOKEN_KEYWORD_TABLE},
    {"DELETE",              TokenType::TOKEN_KEYWORD_DELETE},
    {"UPDATE",              TokenType::TOKEN_KEYWORD_UPDATE},
    {"SET",                 TokenType::TOKEN_KEYWORD_SET},
    {"BEGIN",               TokenType::TOKEN_KEYWORD_BEGIN},
    {"COMMIT",              TokenType::TOKEN_KEYWORD_COMMIT},
    {"ROLLBACK",            TokenType::TOKEN_KEYWORD_ROLLBACK},
    {"INDEX",               TokenType::TOKEN_KEYWORD_INDEX},
    {"ON",                  TokenType::TOKEN_KEYWORD_ON},
    {"NULL",                TokenType::TOKEN_KEYWORD_NULL},

    // Data types
    {"INT",                 TokenType::TOKEN_KEYWORD_INT},
    {"TEXT",                TokenType::TOKEN_KEYWORD_TEXT},

    // Operators
    {"AND",                 TokenType::TOKEN_OPERATOR_AND},
    {"OR",                  TokenType::TOKEN_OPERATOR_OR},
    {"NOT",                 TokenType::TOKEN_OPERATOR_NOT},
    {"LIKE",                TokenType::TOKEN_OPERATOR_LIKE},
    {"IN",                  TokenType::TOKEN_OPERATOR_IN},
    {"BETWEEN",             TokenType::TOKEN_OPERATOR_BETWEEN},
    {"IS",                  TokenType::TOKEN_OPERATOR_IS},

    // Literals
    {"TRUE",                TokenType::TOKEN_LITERAL_BOOL_TRUE},
    {"FALSE",               TokenType::TOKEN_LITERAL_BOOL_FALSE},
};

// Constructor
Lexer::Lexer(const std::string &source) {
    this -> source_ = source;
    this -> position_ = 0;
    this -> line_ = 1;
    this -> column_ = 1;
}

char Lexer::peek() const {
    if (isAtEnd()) {
        return '\0';
    }
    return source_[position_];
}

char Lexer::peekNext() const {
    if (position_ + 1 >= source_.size()) {
        return '\0';
    }
    return source_[position_ + 1];
}

char Lexer::advance() {
    char ch = source_[position_++];
    if (ch == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return ch;
}

char Lexer::isAtEnd() const {
    return position_ >= source_.size();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(peek())) {
        advance();
    }
}

void Lexer::skipLineComment() {
    while (!isAtEnd() && (peek() != '\n')) {
        advance();
    }
}

// When actual character is a char, digit or '_'
Token Lexer::readIndefierOrKeyword() {
    int startLine = line_;
    int startColumn = column_;
    std::string keyword;

    // Read while is char, digit or '_'
    while (!isAtEnd() && std::isalnum(peek()) || peek() == '_') {
        keyword += advance();
    }

    // Convert to uppercase
    std::string upp = toUpperCase(keyword);

    auto it = KEYWORDS.find(upp);
    if (it != KEYWORDS.end()) {
        return Token(it -> second, keyword, startLine, startColumn);
    }

    return Token(TokenType::TOKEN_IDENTIFIER, keyword, startLine, startColumn);
}

// Read when the character is a digit
Token Lexer::readNumber() {
    int startLine = line_;
    int startColumn = column_;
    std::string number;

    while (!isAtEnd() && std::isdigit(peek())) {
        number += advance();
    }

    // Check for decimals
    if (!isAtEnd() && peek() == '.' && std::isdigit(peekNext())) {
        number += advance(); // Pass the dot
        while (!isAtEnd() && std::isdigit(peek())) {
            number += advance(); // Here we read the decimals
        }
        return Token(TokenType::TOKEN_LITERAL_FLOAT, number, startLine, startColumn);
    }

    return Token(TokenType::TOKEN_LITERAL_INTEGER, number, startLine, startColumn);
}

// When we find " until we find the closing "
Token Lexer::readString() {
    int startLine = line_;
    int startColumn = column_;

    advance();

    std::string string;

    while (!isAtEnd() && peek() != '"') {
        // We check \" in the string
        if (peek() == '\\' && peekNext() == '"') {
            advance();
            string += advance();
        } else {
            string += advance();
        }
    }

    if (isAtEnd()) {
        throw std::runtime_error("Lexer::readString: End of file");
    }

    advance();

    return Token(TokenType::TOKEN_LITERAL_STRING, string, startLine, startColumn);
}

Token Lexer::readOperatorOrPunctuation() {
    int startLine = line_;
    int startColumn = column_;
    char ch = advance();

    switch (ch) {
        // Punctuation
        case '(':   return Token(TokenType::TOKEN_PUNCTUATION_LPAREN, "(", startLine, startColumn);
        case ')':   return Token(TokenType::TOKEN_PUNCTUATION_RPAREN, ")", startLine, startColumn);
        case ',':   return Token(TokenType::TOKEN_COMMA, ",", startLine, startColumn);
        case ';':   return Token(TokenType::TOKEN_SEMICOLON, ";", startLine, startColumn);
        case '.':   return Token(TokenType::TOKEN_DOT, ".", startLine, startColumn);

        // Operators
        case '+':   return Token(TokenType::TOKEN_OPERATOR_PLUS, "+", startLine, startColumn);
        case '-':   return Token(TokenType::TOKEN_OPERATOR_MINUS, "-", startLine, startColumn);
        case '*':   return Token(TokenType::TOKEN_OPERATOR_MULTIPLY, "*", startLine, startColumn);
        case '/':   return Token(TokenType::TOKEN_OPERATOR_DIVIDE, "/", startLine, startColumn);
        case '=':   return Token(TokenType::TOKEN_OPERATOR_EQUAL, "=", startLine, startColumn);
        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_OPERATOR_NOT_EQUAL, "!=", startLine, startColumn);
            }
            return Token(TokenType::TOKEN_UNKNOWN, "!", startLine, startColumn);
        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_OPERATOR_LESS_THAN_OR_EQUAL, "<=", startLine, startColumn);
            }

            if (peek() == '>') {
                advance();
                return Token(TokenType::TOKEN_OPERATOR_NOT_EQUAL, "<>", startLine, startColumn);
            }
            return Token(TokenType::TOKEN_OPERATOR_LESS_THAN, "<", startLine, startColumn);

        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL, ">=", startLine, startColumn);
            }
            return Token(TokenType::TOKEN_OPERATOR_GREATER_THAN, ">", startLine, startColumn);
        default:
            return Token(TokenType::TOKEN_UNKNOWN, std::string(1, ch), startLine, startColumn);
    }
}

Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return Token(TokenType::TOKEN_EOF, "", line_,column_);
    }

    // Check if its a comment line "--"
    if (peek() == '-' && peekNext() == '-') {
        skipLineComment();
        return nextToken();
    }

    char ch = peek();

    // If it is a letter or _
    if (std::isalpha(ch) || ch == '_') {
        return readIndefierOrKeyword();
    }

    // If it is a digit
    if (std::isdigit(ch)) {
        return readNumber();
    }

    // If it is a literal string ""
    if (ch == '"') {
        return readString();
    }

    return readOperatorOrPunctuation();
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;

    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        if (token.type == TokenType::TOKEN_EOF) {
            break;
        }
    }

    return tokens;
}

std::string Lexer::toUpperCase(const std::string& str) const{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}