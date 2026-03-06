#include "parser/parser.h"
#include <stdexcept>

// Constructor
Parser::Parser(std::vector<Token> tokens) {
    tokens_ = std::move(tokens);
    position_ = 0;
}

// Navigation
Token& Parser::peek() {
    return tokens_[position_];
}

Token& Parser::peekNext() {
    if (position_ + 1 >= tokens_.size()) {
        return tokens_.back();
    }
    return tokens_[position_ + 1];
}

Token &Parser::advance() {
    Token& token = tokens_[position_];
    if (!isAtEnd()) {
        position_++;
    }

    return token;
}

bool Parser::isAtEnd() const {
    return tokens_[position_].type == TokenType::TOKEN_EOF;
}

bool Parser::check(TokenType type) const {
    return tokens_[position_].type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token &Parser::consume(TokenType type, const std::string &errorMessage) {
    if (check(type)) {
        return advance();
    }

    Token& tokenCurrent = peek();
    throw std::runtime_error("Error in line: " + std::to_string(tokenCurrent.line) + " and column: " +
        errorMessage + " (found in: \"" + tokenCurrent.value + "\")");
}

// Expression parsers
ASTNodePointer Parser::parse() {
    if (check(TokenType::TOKEN_KEYWORD_CREATE)) {
        advance();
        return parseCreateTable();
    }
    if (check(TokenType::TOKEN_KEYWORD_INSERT)) {
        advance();
        return parseInsert();
    }
    if (check(TokenType::TOKEN_KEYWORD_SELECT)) {
        advance();
        return parseSelect();
    }
    if (check(TokenType::TOKEN_KEYWORD_DELETE)) {
        advance();
        return parseDelete();
    }
    if (check(TokenType::TOKEN_KEYWORD_UPDATE)) {
        advance();
        return parseUpdate();
    }

    throw std::runtime_error("Error in line: " + std::to_string(peek().line) + " and column: " +
        std::to_string(peek().column) + " (sentence not reconized: \"" + peek().value + "\")");
}

// Create Table
ASTNodePointer Parser::parseCreateTable() {
    // We have already done CREATE, so we need TABLE
    consume(TokenType::TOKEN_KEYWORD_TABLE, "expected TABLE after CREATE");

    // Table name
    Token& tokenName = consume(TokenType::TOKEN_IDENTIFIER, "expected table name");
    // equivalent: std::unique_ptr<CreateTableNode> node(new CreateTableNode());
    auto node = std::make_unique<CreateTableNode>();
    node->tableName = tokenName.value;

    // Open parenthesis
    consume(TokenType::TOKEN_PUNCTUATION_LPAREN, "expected '('");

    // Read values
    while (!isAtEnd() && !check(TokenType::TOKEN_PUNCTUATION_RPAREN)) {
        // Column name
        Token& columnName = consume(TokenType::TOKEN_IDENTIFIER, "expected column name");

        // Column type
        TokenType columnType;
        if (check(TokenType::TOKEN_KEYWORD_INT)) {
            columnType = TokenType::TOKEN_KEYWORD_INT;
            advance();
        } else if (check(TokenType::TOKEN_KEYWORD_TEXT)) {
            columnType = TokenType::TOKEN_KEYWORD_TEXT;
            advance();
        } else {
            throw std::runtime_error("Error in line: "+ std::to_string(peek().line) + " and column: " +
                std::to_string(peek().column) + " ( column type not valid : \"" + peek().value + "\" expected INT or TEXT)");
        }

        node->columns.push_back({columnName.value, columnType});

        if (!match(TokenType::TOKEN_COMMA)) {
            break;
        }
    }
    // Close parenthesis
    consume(TokenType::TOKEN_PUNCTUATION_RPAREN, "expected ')'");
    // Optional semicolon
    match(TokenType::TOKEN_SEMICOLON);

    return node;
}

// Insert Into
ASTNodePointer Parser::parseInsert() {
    // We have already done INSERT, we need INTO
    consume(TokenType::TOKEN_KEYWORD_INTO, "expected INTO after INSERT");

    // Token name
    Token& tokenName = consume(TokenType::TOKEN_IDENTIFIER, "expected table name");
    auto node = std::make_unique<InsertNode>();
    node->tableName = tokenName.value;

    // Values
    consume(TokenType::TOKEN_KEYWORD_VALUES, "expected VALUES");
    // Open parenthesis
    consume(TokenType::TOKEN_PUNCTUATION_LPAREN, "expected '('");

    // Read values
    while (!isAtEnd() && !check(TokenType::TOKEN_PUNCTUATION_RPAREN)) {
        node->values.push_back(parsePrimary());

        if (!match(TokenType::TOKEN_COMMA)) {
            break;
        }
    }

    consume(TokenType::TOKEN_PUNCTUATION_RPAREN, "expected ')'");
    match(TokenType::TOKEN_SEMICOLON);

    return node;
}

// Parse Select
ASTNodePointer Parser::parseSelect() {
    auto node = std::make_unique<SelectNode>();

    // Reads columns: * or id list
    if (match(TokenType::TOKEN_OPERATOR_MULTIPLY)) {
        // SELECT *
        node->columns.push_back("*");
    } else {
        Token& column = consume(TokenType::TOKEN_IDENTIFIER, "expected column name");
        node->columns.push_back(column.value);

        while (match(TokenType::TOKEN_COMMA)) {
            Token& column = consume(TokenType::TOKEN_IDENTIFIER, "expected column name");
            node->columns.push_back(column.value);
        }
    }

    // FROM
    consume(TokenType::TOKEN_KEYWORD_FROM, "expected FROM after SELECT");

    // Table name
    Token& tokenName = consume(TokenType::TOKEN_IDENTIFIER, "expected table name");
    node->tableName = tokenName.value;

    // WHERE
    if (match(TokenType::TOKEN_KEYWORD_WHERE)) {
        node->where = parseExpression();
    }

    // Optional semicolon
    match(TokenType::TOKEN_SEMICOLON);

    return node;
}

// Parse delete
ASTNodePointer Parser::parseDelete() {}