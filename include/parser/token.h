// token.h
#ifndef TOKEN_H
#define TOKEN_H

// Basic ennumeration of SQL tokens
typedef enum {
    TOKEN_OPERATOR_EQUAL,   // =
    TOKEN_OPERATOR_DIFFERENT, // !=
    TOKEN_OPERATOR_NOT_EQUAL,   // <>
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
    TOKEN_OPERATOR_IS_NULL  // IS NULL
} TokenType;

// Token Structure
typedef struct {
    TokenType type; // Token type
    char * lexeme;  // string that represents token
} Token;

#endif // TOKEN_H
