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

        if (check(TokenType::TOKEN_KEYWORD_INDEX)) {
            advance();
            return parseCreateIndex();
        }
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
    if (check(TokenType::TOKEN_KEYWORD_BEGIN)) {
        advance();
        match(TokenType::TOKEN_SEMICOLON);
        return std::make_unique<BeginNode>();
    }
    if (check(TokenType::TOKEN_KEYWORD_COMMIT)) {
        advance();
        match(TokenType::TOKEN_SEMICOLON);
        return std::make_unique<CommitNode>();
    }
    if (check(TokenType::TOKEN_KEYWORD_ROLLBACK)) {
        advance();
        match(TokenType::TOKEN_SEMICOLON);
        return std::make_unique<RollbackNode>();
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
                std::to_string(peek().column) + " ( column type not valid : \"" + peek().value +
                "\" expected INT or TEXT)");
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
ASTNodePointer Parser::parseDelete() {
    // We have already done DELETE, we need FROM
    consume(TokenType::TOKEN_KEYWORD_FROM, "expected FROM after DELETE");

    // Table name
    Token& tokenName = consume(TokenType::TOKEN_IDENTIFIER, "expected table name");
    auto node = std::make_unique<DeleteNode>();
    node->tableName = tokenName.value;

    // WHERE (optional)
    if (match(TokenType::TOKEN_KEYWORD_WHERE)) {
        node->where = parseExpression();
    }

    // Semicolon optional
    match(TokenType::TOKEN_SEMICOLON);

    return node;
}

// Parse Update
ASTNodePointer Parser::parseUpdate() {
    // Table name
    Token& tokenName = consume(TokenType::TOKEN_IDENTIFIER, "expected table name");
    auto node = std::make_unique<UpdateNode>();
    node->tableName = tokenName.value;

    // SET
    consume(TokenType::TOKEN_KEYWORD_SET, "expected SET");

    // Read assignment (col = value, col = value...)
    while (!isAtEnd()) {
        Token& columnName = consume(TokenType::TOKEN_IDENTIFIER, "expected column name");
        consume(TokenType::TOKEN_OPERATOR_EQUAL, "expected '='");
        ASTNodePointer value = parsePrimary();

        UpdateNode::Assignment assignment;
        assignment.column = columnName.value;
        assignment.value = std::move(value);
        node->assignments.push_back(std::move(assignment));

        if (!match(TokenType::TOKEN_COMMA)) {
            break;
        }
    }

    if (match(TokenType::TOKEN_KEYWORD_WHERE)) {
        node->where = parseExpression();
    }

    match(TokenType::TOKEN_SEMICOLON);

    return node;
}

// EXPRESION PARSERS
// Precedence
    // OR less precedence   -> parse first
    // AND more precedence  -> parse after
    // Comparations         -> parse at the end

// Parse Expressions

// Level OR
ASTNodePointer Parser::parseExpression() {
    ASTNodePointer left = parseAnd();

    while (check(TokenType::TOKEN_OPERATOR_OR)) {
        TokenType op = peek().type;
        advance();
        ASTNodePointer right = parseAnd();
        ASTNodePointer newNode = std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
        left = std::move(newNode);
    }

    return left;
}

// Level AND
ASTNodePointer Parser::parseAnd() {
    ASTNodePointer left = parseComparison();

    while (check(TokenType::TOKEN_OPERATOR_AND)) {
        TokenType op = peek().type;
        advance();
        ASTNodePointer right = parseComparison();
        left = std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
    }

    return left;
}

// Level comparation
ASTNodePointer Parser::parseComparison() {
    ASTNodePointer left = parsePrimary();

    if (check(TokenType::TOKEN_OPERATOR_EQUAL) || check(TokenType::TOKEN_OPERATOR_NOT_EQUAL) ||
        check(TokenType::TOKEN_OPERATOR_LESS_THAN) || check(TokenType::TOKEN_OPERATOR_GREATER_THAN) ||
        check(TokenType::TOKEN_OPERATOR_LESS_THAN_OR_EQUAL) || check(TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL))
        {
            TokenType op = peek().type;
            advance();
            ASTNodePointer right = parsePrimary();
            return std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
    }

    return left;
}

// Level primary
ASTNodePointer Parser::parsePrimary() {
    // Name of the column
    if (check(TokenType::TOKEN_IDENTIFIER)) {
        std::string name = peek().value;
        advance();
        return std::make_unique<IdentifyNode>(name);
    }

    // Int
    if (check(TokenType::TOKEN_LITERAL_INTEGER)) {
        int value = std::stoi(peek().value);
        advance();
        return std::make_unique<IntLiteralNode>(value); // str to int
    }

    // Float
    if (check(TokenType::TOKEN_LITERAL_FLOAT)) {
        float value = std::stof(peek().value);
        advance();
        return std::make_unique<FloatLiteralNode>(value);
    }

    // String
    if (check(TokenType::TOKEN_LITERAL_STRING)) {
        std::string value = peek().value;
        advance();
        return std::make_unique<StringLiteralNode>(value);
    }

    // Boolean TRUE
    if (check(TokenType::TOKEN_LITERAL_BOOL_TRUE)) {
        advance();
        return std::make_unique<BoolLiteralNode>(true);
    }

    // Boolean FALSE
    if (check(TokenType::TOKEN_LITERAL_BOOL_FALSE)) {
        advance();
        return std::make_unique<BoolLiteralNode>(false);
    }

    throw std::runtime_error( "Error in line: " + std::to_string(peek().line) + " and in column: " +
        std::to_string(peek().column) + " (value no expected: \"" + peek().value + "\"");
}

ASTNodePointer Parser::parseCreateIndex() {
    // CREATE INDEX name ON TABLE
    Token& indexName = consume(TokenType::TOKEN_IDENTIFIER, "expected index name");
    consume(TokenType::TOKEN_KEYWORD_ON, "expected ON");
    Token& tableName = consume(TokenType::TOKEN_IDENTIFIER, "expected table name");
    consume(TokenType::TOKEN_PUNCTUATION_LPAREN, "expected '(' ");
    Token& columnName = consume(TokenType::TOKEN_IDENTIFIER, "expected column name");
    consume(TokenType::TOKEN_PUNCTUATION_RPAREN, "expected ')'");

    match(TokenType::TOKEN_SEMICOLON);

    auto node = std::make_unique<CreateIndexNode>();
    node->indexName = indexName.value;
    node->tableName = tableName.value;
    node->columnName = columnName.value;

    return node;
}