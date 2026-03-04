// lexer.cpp
#include "../../include/parser/token.h"

// TokenType to String
std::string tokenTypeName(TokenType type) {
    switch (type) {
        // Keywords
        case TokenType::TOKEN_KEYWORD_SELECT:                   return "KEYWORD_SELECT";
        case TokenType::TOKEN_KEYWORD_FROM:                     return "KEYWORD_FROM";
        case TokenType::TOKEN_KEYWORD_WHERE:                    return "KEYWORD_WHERE";
        case TokenType::TOKEN_KEYWORD_INSERT:                   return "KEYWORD_INSERT";
        case TokenType::TOKEN_KEYWORD_INTO:                     return "KEYWORD_INTO";
        case TokenType::TOKEN_KEYWORD_VALUES:                   return "KEYWORD_VALUES";
        case TokenType::TOKEN_KEYWORD_CREATE:                   return "KEYWORD_CREATE";
        case TokenType::TOKEN_KEYWORD_TABLE:                    return "KEYWORD_TABLE";
        case TokenType::TOKEN_KEYWORD_DELETE:                   return "KEYWORD_DELETE";
        case TokenType::TOKEN_KEYWORD_UPDATE:                   return "KEYWORD_UPDATE";
        case TokenType::TOKEN_KEYWORD_SET:                      return "KEYWORD_SET";
        case TokenType::TOKEN_KEYWORD_BEGIN:                    return "KEYWORD_BEGIN";
        case TokenType::TOKEN_KEYWORD_COMMIT:                   return "KEYWORD_COMMIT";
        case TokenType::TOKEN_KEYWORD_ROLLBACK:                 return "KEYWORD_ROLLBACK";
        case TokenType::TOKEN_KEYWORD_INDEX:                    return "KEYWORD_INDEX";
        case TokenType::TOKEN_KEYWORD_ON:                       return "KEYWORD_ON";
        case TokenType::TOKEN_KEYWORD_NULL:                     return "KEYWORD_NULL";

        // Data types
        case TokenType::TOKEN_KEYWORD_INT:                      return "KEYWORD_INT";
        case TokenType::TOKEN_KEYWORD_TEXT:                     return "KEYWORD_TEXT";

        // Operators
        case TokenType::TOKEN_OPERATOR_EQUAL:                   return "OPERATOR_EQUAL";
        case TokenType::TOKEN_OPERATOR_NOT_EQUAL:               return "OPERATOR_NOT_EQUAL";
        case TokenType::TOKEN_OPERATOR_LESS_THAN:               return "OPERATOR_LESS_THAN";
        case TokenType::TOKEN_OPERATOR_GREATER_THAN:            return "OPERATOR_GREATER_THAN";
        case TokenType::TOKEN_OPERATOR_LESS_THAN_OR_EQUAL:      return "OPERATOR_LESS_THAN_OR_EQUAL";
        case TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL:   return "OPERATOR_GREATER_THAN_OR_EQUAL";
        case TokenType::TOKEN_OPERATOR_AND:                     return "OPERATOR_AND";
        case TokenType::TOKEN_OPERATOR_OR:                      return "OPERATOR_OR";
        case TokenType::TOKEN_OPERATOR_NOT:                     return "OPERATOR_NOT";
        case TokenType::TOKEN_OPERATOR_PLUS:                    return "OPERATOR_PLUS";
        case TokenType::TOKEN_OPERATOR_MINUS:                   return "OPERATOR_MINUS";
        case TokenType::TOKEN_OPERATOR_MULTIPLY:                return "OPERATOR_MULTIPLY";
        case TokenType::TOKEN_OPERATOR_DIVIDE:                  return "OPERATOR_DIVIDE";
        case TokenType::TOKEN_OPERATOR_LIKE:                    return "OPERATOR_LIKE";
        case TokenType::TOKEN_OPERATOR_IN:                      return "OPERATOR_IN";
        case TokenType::TOKEN_OPERATOR_BETWEEN:                 return "OPERATOR_BETWEEN";
        case TokenType::TOKEN_OPERATOR_IS:                      return "OPERATOR_IS";

        // Literals
        case TokenType::TOKEN_LITERAL_INTEGER:                  return "LITERAL_INTEGER";
        case TokenType::TOKEN_LITERAL_FLOAT:                    return "LITERAL_FLOAT";
        case TokenType::TOKEN_LITERAL_STRING:                   return "LITERAL_STRING";
        case TokenType::TOKEN_LITERAL_BOOL_TRUE:                return "BOOL_TRUE";
        case TokenType::TOKEN_LITERAL_BOOL_FALSE:               return "BOOL_FALSE";

        // Identifier
        case TokenType::TOKEN_IDENTIFIER:                       return "IDENTIFIER";

        // Punctuation
        case TokenType::TOKEN_PUNCTUATION_LPAREN:               return "PUNCTUATION_LPAREN";
        case TokenType::TOKEN_PUNCTUATION_RPAREN:               return "PUNCTUATION_RPAREN";
        case TokenType::TOKEN_COMMA:                            return "COMMA";
        case TokenType::TOKEN_SEMICOLON:                        return "SEMICOLON";
        case TokenType::TOKEN_DOT:                              return "DOT";

        // Special
        case TokenType::TOKEN_EOF:                              return "EOF";
        case TokenType::TOKEN_UNKNOWN:                          return "UNKNOWN";

        default:                                                return "Error";
    }
}

std::string Token::toString() const {
    return "[" + tokenTypeName(type) +
           " | \"" + value + "\"" +
           " | L" + std::to_string(line) +
           ":C" + std::to_string(column) + "]";
}