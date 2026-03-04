// token.h
// Lexic unity

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Basic ennumeration of SQL tokens
enum class TokenType {
    // KEYWORDS
    TOKEN_KEYWORD_SELECT,   // SELECT
    TOKEN_KEYWORD_FROM,     // FROM
    TOKEN_KEYWORD_WHERE,    // WHERE
    TOKEN_KEYWORD_INSERT,   // INSERT
    TOKEN_KEYWORD_INTO,     // INTO
    TOKEN_KEYWORD_VALUES,   // VALUES
    TOKEN_KEYWORD_CREATE,   // CREATE
    TOKEN_KEYWORD_TABLE,    // TABLE
    TOKEN_KEYWORD_DELETE,   // DELETE
    TOKEN_KEYWORD_UPDATE,   // UPDATE
    TOKEN_KEYWORD_SET,  // SET
    TOKEN_KEYWORD_BEGIN,    // BEGIN
    TOKEN_KEYWORD_COMMIT,   // COMMIT
    TOKEN_KEYWORD_ROLLBACK, // ROLLBACK
    TOKEN_KEYWORD_INDEX,    // INDEX
    TOKEN_KEYWORD_ON,   // ON
    TOKEN_KEYWORD_NULL, // NULL

    // "DATA TYPES"
    TOKEN_KEYWORD_INT,  // INT
    TOKEN_KEYWORD_TEXT, // TEXT

    // OPERATORS
    TOKEN_OPERATOR_EQUAL,   // =
    TOKEN_OPERATOR_NOT_EQUAL,   // <> or !=
    TOKEN_OPERATOR_LESS_THAN,   // <
    TOKEN_OPERATOR_GREATER_THAN, // >
    TOKEN_OPERATOR_LESS_THAN_OR_EQUAL,  // <=
    TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL, // >=
    TOKEN_OPERATOR_AND, // AMD
    TOKEN_OPERATOR_OR,  // OR
    TOKEN_OPERATOR_NOT, // NOT
    TOKEN_OPERATOR_PLUS, // +
    TOKEN_OPERATOR_MINUS,   // -
    TOKEN_OPERATOR_MULTIPLY,    // *
    TOKEN_OPERATOR_DIVIDE,  // /
    TOKEN_OPERATOR_LIKE, // LIKE
    TOKEN_OPERATOR_IN,  // IN
    TOKEN_OPERATOR_BETWEEN, // BETWEEN
    TOKEN_OPERATOR_IS,  // IS

    // LITERALS
    TOKEN_LITERAL_INTEGER,
    TOKEN_LITERAL_FLOAT,
    TOKEN_LITERAL_STRING,
    TOKEN_LITERAL_BOOL_TRUE,    // TRUE
    TOKEN_LITERAL_BOOL_FALSE,   // FALSE

    // IDENTIFIER
    TOKEN_IDENTIFIER,

    // PUNCTUATION
    TOKEN_PUNCTUATION_LPAREN,   // (
    TOKEN_PUNCTUATION_RPAREN,   // )
    TOKEN_COMMA,    // ,
    TOKEN_SEMICOLON,    // ;
    TOKEN_DOT,  // .

    // SPCECIAL
    TOKEN_EOF,
    TOKEN_UNKNOWN,
};

// Token Structure
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    // Principal constructor
    Token(TokenType type, std::string value, int line, int column) {
        this->type = type;
        this->value = value;
        this->line = line;
        this->column = column;
    }

    // Default constructor
    Token() {
        this->type = TokenType::TOKEN_UNKNOWN;
        this->value = "";
        this->line = 0;
        this->column = 0;
    }

    // Methods
    // Comprovem que sigui keyword
    bool isKeyword() const {
        return type >= TokenType::TOKEN_KEYWORD_SELECT && type <= TokenType::TOKEN_KEYWORD_TEXT;
    }

    // Comprovem que es operator
    bool isOperator() const {
        return type >= TokenType::TOKEN_OPERATOR_AND && type <= TokenType::TOKEN_OPERATOR_DIVIDE;
    }

    // Comprovem si és literal
    bool isLiteral() const {
        return type >= TokenType::TOKEN_LITERAL_INTEGER && type <= TokenType::TOKEN_LITERAL_BOOL_FALSE;
    }

    std::string toString() const;
};

std::string tokenTypeName(TokenType type);

#endif // TOKEN_H
