#include "parser/lexer.h"
#include "parser/token.h"

#include <iostream>
#include <string>
#include <vector>

void printTokens(const std::string& str) {
    std::cout << "SQL statement: " <<str << std::endl;

    Lexer lexer(str);
    auto tokens = lexer.tokenizeAll();

    for (const auto& token : tokens) {
        std::cout << " " << token.toString() << std::endl;
    }
}

void assertTokensTypes(const std::string& str, const std::vector<TokenType>& expected, const std::string& test) {
    Lexer lexer(str);
    auto tokens = lexer.tokenizeAll();

    if (tokens.size() != expected.size()) {
        std::cout << "FAIL: " << test << std::endl;
        std::cout << "Expected: " << expected.size() << std::endl;
        std::cout << "Actual: " << tokens.size() << std::endl;
        std::cout << "SQL statement: " << str << std::endl;
        return;
    }

    for (size_t i = 0; i < expected.size(); i++) {
        if (tokens[i].type != expected[i]) {
            std::cout << "FAIL: " << test << std::endl;
            std::cout << "Expected: " << tokenTypeName(expected[i]) << std::endl;
            std::cout << "Actual: " << tokens[i].toString() << std::endl;
            std::cout << "SQL statement: " << str << std::endl;
            return;
        }
    }

    std::cout << "SUCCESS: " << test << std::endl;
}

// TESTS 1
void testSelectSimple() {
    assertTokensTypes("SELECT name FROM users;",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "SELECT simple"
        );
}

// TEST 2
void testSelectWhere() {
    assertTokensTypes("SELECT * FROM products WHERE prices > 100;",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_OPERATOR_MULTIPLY,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_GREATER_THAN,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "SELECT whith WHERE and >"
        );
}

// TEST 3
void testCreateTable() {
    assertTokensTypes("CREATE TABLE users (id INT, name TEXT);",
        {
            TokenType::TOKEN_KEYWORD_CREATE,
            TokenType::TOKEN_KEYWORD_TABLE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_PUNCTUATION_LPAREN,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_INT,
            TokenType::TOKEN_COMMA,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_TEXT,
            TokenType::TOKEN_PUNCTUATION_RPAREN,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "CREATE TABLE;"
        );
}

// TEST 4
void testInsert() {
    assertTokensTypes("INSERT INTO users VALUES (1, \"Ana\");",
        {
            TokenType::TOKEN_KEYWORD_INSERT,
            TokenType::TOKEN_KEYWORD_INTO,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_VALUES,
            TokenType::TOKEN_PUNCTUATION_LPAREN,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_COMMA,
            TokenType::TOKEN_LITERAL_STRING,
            TokenType::TOKEN_PUNCTUATION_RPAREN,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "INSERT INTO"
        );
}

// TEST 5
void testDelete() {
    assertTokensTypes(
        "DELETE FROM users WHERE id = 3;",
        {
            TokenType::TOKEN_KEYWORD_DELETE,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF
        },
        "DELETE with WHERE"
        );
}

// TEST 6
void testComparation() {
    assertTokensTypes( "SELECT * FROM t WHERE a >= 10 AND b != 20 AND c <= 30;",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_OPERATOR_MULTIPLY,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_OPERATOR_AND,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_NOT_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_OPERATOR_AND,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_LESS_THAN_OR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "Operators  >=, !=, <="
        );
}

// TEST 7
void testFloat() {
    assertTokensTypes("SELECT price FROM products WHERE discount >= 3.14;",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "Literal float"
        );
}

// TEST 8
void testUpdate() {
    assertTokensTypes("UPDATE users SET name = \"John\", age = 20 WHERE id = 1;",
        {
            TokenType::TOKEN_KEYWORD_UPDATE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_SET,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_EQUAL,
            TokenType::TOKEN_LITERAL_STRING,
            TokenType::TOKEN_COMMA,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "UPDATE SET"
        );
}

// TEST 9
void testTransaction() {
    assertTokensTypes("BEGIN; INSERT INTO t VALUES (1, \"x\"); COMMIT;",
        {
            TokenType::TOKEN_KEYWORD_BEGIN,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_KEYWORD_INSERT,
            TokenType::TOKEN_KEYWORD_INTO,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_VALUES,
            TokenType::TOKEN_PUNCTUATION_LPAREN,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_COMMA,
            TokenType::TOKEN_LITERAL_STRING,
            TokenType::TOKEN_PUNCTUATION_RPAREN,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_KEYWORD_COMMIT,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "BEGIN / COMMIT"
        );
}

// TEST 10
void testComment() {
    assertTokensTypes("SELECT id FROM t; -- this is a comment",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "Ignore comment"
        );
}

// TEST 11
void testCaseInsensitive() {
    assertTokensTypes("select * from USERS where ID = 1",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_OPERATOR_MULTIPLY,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_EQUAL,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "Keywords insesitive case"
        );
}

// TEST 12
void testOperatorsSQL() {
    assertTokensTypes("SELECT * FROM t WHERE name LIKE \"A%\" AND age BETWEEN 18 AND 65;",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_OPERATOR_MULTIPLY,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_LIKE,
            TokenType::TOKEN_LITERAL_STRING,
            TokenType::TOKEN_OPERATOR_AND,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_BETWEEN,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_OPERATOR_AND,
            TokenType::TOKEN_LITERAL_INTEGER,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "LIKE and BETWEEN"
        );
}

// TEST 13
void testDifferentsOperators() {
    assertTokensTypes("SELECT * FROM t WHERE state <> \"inactive\";",
        {
            TokenType::TOKEN_KEYWORD_SELECT,
            TokenType::TOKEN_OPERATOR_MULTIPLY,
            TokenType::TOKEN_KEYWORD_FROM,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_WHERE,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_OPERATOR_NOT_EQUAL,
            TokenType::TOKEN_LITERAL_STRING,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "Operator <>"
        );
}

// TEST 14
void testBooleans() {
    assertTokensTypes("INSERT INTO config VALUES (TRUE, FALSE);",
        {
            TokenType::TOKEN_KEYWORD_INSERT,
            TokenType::TOKEN_KEYWORD_INTO,
            TokenType::TOKEN_IDENTIFIER,
            TokenType::TOKEN_KEYWORD_VALUES,
            TokenType::TOKEN_PUNCTUATION_LPAREN,
            TokenType::TOKEN_LITERAL_BOOL_TRUE,
            TokenType::TOKEN_COMMA,
            TokenType::TOKEN_LITERAL_BOOL_FALSE,
            TokenType::TOKEN_PUNCTUATION_RPAREN,
            TokenType::TOKEN_SEMICOLON,
            TokenType::TOKEN_EOF,
        },
        "Literals TRUE / FALSE"
        );
}

int main() {
    std::cout << "START TESTS " << std::endl;

    testSelectSimple();
    testSelectWhere();
    testCreateTable();
    testInsert();
    testDelete();
    testComparation();
    testFloat();
    testUpdate();
    testTransaction();
    testComment();
    testCaseInsensitive();
    testOperatorsSQL();
    testDifferentsOperators();
    testBooleans();

    std::cout << "END TESTS " << std::endl;

    return 0;
}