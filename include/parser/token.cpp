// lexer.cpp
#include "token.h"

// TokenType to String
std::string tokenTypeName(TokenType type) {
    switch (type) {
        case
    }
}

std::string Token::toString() const {
    return "[" + tokenTypeName(type) +
           " | \"" + value + "\"" +
           " | L" + std::to_string(line) +
           ":C" + std::to_string(column) + "]";
}