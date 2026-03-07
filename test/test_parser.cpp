#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/ast.h"
#include <iostream>
#include <string>
#include <vector>

int passed = 0;
int failed = 0;

// Parse a query and return AST as a String (if fail throw exception)
std::string parseToString(const std::string& sql) {
    Lexer lexer(sql);
    auto tokens = lexer.tokenizeAll();
    Parser parser(std::move(tokens));
    auto ast = parser.parse();
    return ast->toString();
}

// Check query parse with no errors
void assertParses(const std::string& sql, const std::string& testName) {
    try {
        parseToString(sql);
        std::cout << " PASS [" << testName << "]\n";
        passed++;
    } catch (const std::exception& e) {
        std::cout << " FAIL [" << testName << "]\n";
        std::cout << "   ERROR: " << e.what() << "\n";
        std::cout << "   SQL: " << sql << "\n";
        failed++;
    }
}

// Check query fail with error
void assertFails(const std::string& sql, const std::string& testName) {
    try {
        parseToString(sql);
        std::cout << " FAIL [" << testName << "] — expected\n";
        std::cout << "   SQL: " << sql << "\n";
        failed++;
    } catch (const std::exception&) {
        std::cout << " PASS [" << testName << "] (error expected)\n";
        passed++;
    }
}

// Check AST has excpected string
void assertASTContains(const std::string& sql,
                       const std::string& expected,
                       const std::string& testName) {
    try {
        std::string ast = parseToString(sql);
        if (ast.find(expected) != std::string::npos) {
            std::cout << " PASS [" << testName << "]\n";
            passed++;
        } else {
            std::cout << " FAIL [" << testName << "]\n";
            std::cout << "   Expected is AST: \"" << expected << "\"\n";
            std::cout << "   AST obtained:\n" << ast << "\n";
            std::cout << "   SQL: " << sql << "\n";
            failed++;
        }
    } catch (const std::exception& e) {
        std::cout << " FAIL [" << testName << "]\n";
        std::cout << "   ERROR: " << e.what() << "\n";
        failed++;
    }
}

// TESTS CREATE TABLE
void testCreateTable() {
    // Basic
    assertParses(
        "CREATE TABLE users (id INT, name TEXT);",
        "CREATE TABLE basic"
    );

    // Only one column
    assertParses(
        "CREATE TABLE t (id INT);",
        "CREATE TABLE one column"
    );

    // More columns
    assertParses(
        "CREATE TABLE products (id INT, name TEXT, price TEXT, stock INT);",
        "CREATE TABLE lot of columns"
    );

    // Name of table appears in AST
    assertASTContains(
        "CREATE TABLE clients (id INT);",
        "clients",
        "CREATE TABLE correct name in AST"
    );

    // INT type appears in AST
    assertASTContains(
        "CREATE TABLE t (age INT);",
        "KEYWORD_INT",
        "CREATE TABLE type INT in AST"
    );

    // TEXT type appears in AST
    assertASTContains(
        "CREATE TABLE t (name TEXT);",
        "KEYWORD_TEXT",
        "CREATE TABLE type TEXT in AST"
    );

    // ERROR: without parenthesis
    assertFails(
        "CREATE TABLE t id INT;",
        "CREATE TABLE without parenthesis -> ERROR"
    );

    // ERROR: invalid column type
    assertFails(
        "CREATE TABLE t (id FLOAT);",
        "CREATE TABLE invalid type -> ERROR"
    );
}

// TESTS INSERT
void testInsert() {
    // Basic with int and string
    assertParses(
        "INSERT INTO users VALUES (1, \"John\");",
        "INSERT basic"
    );

    // A lot of values
    assertParses(
        "INSERT INTO users VALUES (1, \"John\", 25);",
        "INSERT multiple values"
    );

    // With float
    assertParses(
        "INSERT INTO products VALUES (1, \"Laptop\", 999.99);",
        "INSERT with float"
    );

    // With boolean
    assertParses(
        "INSERT INTO config VALUES (TRUE, FALSE);",
        "INSERT with booleans"
    );

    // Table name appears in AST
    assertASTContains(
        "INSERT INTO cart VALUES (1, \"test\");",
        "cart",
        "INSERT table name in AST"
    );

    // String value appears in AST
    assertASTContains(
        "INSERT INTO t VALUES (1, \"Mary\");",
        "Mary",
        "INSERT literal string in AST"
    );

    // ERROR: without INTO
    assertFails(
        "INSERT users VALUES (1);",
        "INSERT without INTO -> ERROR"
    );

    // ERROR: without values
    assertFails(
        "INSERT INTO users (1);",
        "INSERT without VALUES -> ERROR"
    );
}

// TESTS SELECT
void testSelect() {
    // SELECT *
    assertParses(
        "SELECT * FROM users;",
        "SELECT *"
    );

    // SELECT simple column
    assertParses(
        "SELECT name FROM users;",
        "SELECT one column"
    );

    // SELECT multiple columns
    assertParses(
        "SELECT name, age, id FROM users;",
        "SELECT multiple columns"
    );

    // SELECT with WHERE simple
    assertParses(
        "SELECT * FROM users WHERE id = 1;",
        "SELECT with WHERE ="
    );

    // SELECT with WHERE and >
    assertParses(
        "SELECT * FROM products WHERE price > 100;",
        "SELECT with WHERE >"
    );

    // SELECT with WHERE and AND
    assertParses(
        "SELECT * FROM users WHERE age > 18 AND active = TRUE;",
        "SELECT with WHERE AND"
    );

    // SELECT with WHERE and OR
    assertParses(
        "SELECT * FROM t WHERE a = 1 OR b = 2;",
        "SELECT with WHERE OR"
    );

    // SELECT with WHERE complex: AND and OR
    assertParses(
        "SELECT * FROM t WHERE a > 1 AND b < 5 OR c = 3;",
        "SELECT with WHERE AND and OR"
    );

    // Table name appears in AST
    assertASTContains(
        "SELECT * FROM inventory;",
        "inventory",
        "SELECT table name in AST"
    );

    // WHERE column appears in AST
    assertASTContains(
        "SELECT * FROM t WHERE age > 20;",
        "age",
        "SELECT column WHERE in AST"
    );

    // ERROR: without FROM
    assertFails(
        "SELECT * users;",
        "SELECT without FROM -> ERROR"
    );
}

// TESTS DELETE
void testDelete() {
    // DELETE without WHERE
    assertParses(
        "DELETE FROM users;",
        "DELETE without WHERE"
    );

    // DELETE with WHERE
    assertParses(
        "DELETE FROM users WHERE id = 1;",
        "DELETE with WHERE"
    );

    // DELETE with WHERE complex
    assertParses(
        "DELETE FROM products WHERE price < 10 AND stock = 0;",
        "DELETE with WHERE AND"
    );

    // Table name appears in AST
    assertASTContains(
        "DELETE FROM chart WHERE id = 5;",
        "chart",
        "DELETE nombre tabla en AST"
    );

    // ERROR: without FROM
    assertFails(
        "DELETE users WHERE id = 1;",
        "DELETE without FROM -> ERROR"
    );
}

// TESTS UPDATE
void testUpdate() {
    // Basic UPDATE
    assertParses(
        "UPDATE users SET name = \"Louis\";",
        "Basic UPDATE"
    );

    // UPDATE multiples columns
    assertParses(
        "UPDATE users SET name = \"Louis\", age = 30;",
        "UPDATE multiples columns"
    );

    // UPDATE with WHERE
    assertParses(
        "UPDATE users SET name = \"Louis\" WHERE id = 1;",
        "UPDATE with WHERE"
    );

    // UPDATE multiple columns with WHERE
    assertParses(
        "UPDATE users SET names = \"John\", age = 25 WHERE id = 1;",
        "UPDATE multiple columns with WHERE"
    );

    // Table names appears in AST
    assertASTContains(
        "UPDATE clients SET name = \"test\" WHERE id = 1;",
        "clients",
        "UPDATE table name in AST"
    );

    // ERROR: without SET
    assertFails(
        "UPDATE users name = \"Louis\";",
        "UPDATE without SET -> ERROR"
    );
}

// TESTS WHERE — expressions and precedence
void testWhere() {
    // All comparation operators
    assertParses("SELECT * FROM t WHERE a = 1;",  "WHERE =");
    assertParses("SELECT * FROM t WHERE a != 1;", "WHERE !=");
    assertParses("SELECT * FROM t WHERE a < 1;",  "WHERE <");
    assertParses("SELECT * FROM t WHERE a > 1;",  "WHERE >");
    assertParses("SELECT * FROM t WHERE a <= 1;", "WHERE <=");
    assertParses("SELECT * FROM t WHERE a >= 1;", "WHERE >=");

    // With literal string
    assertParses(
        "SELECT * FROM t WHERE name = \"John\";",
        "WHERE with string"
    );

    // With float
    assertParses(
        "SELECT * FROM t WHERE price >= 9.99;",
        "WHERE with float"
    );

    // With boolean
    assertParses(
        "SELECT * FROM t WHERE active = TRUE;",
        "WHERE with TRUE"
    );

    // AND chain multiple
    assertParses(
        "SELECT * FROM t WHERE a = 1 AND b = 2 AND c = 3;",
        "WHERE triple AND"
    );

    // OR chain multiple
    assertParses(
        "SELECT * FROM t WHERE a = 1 OR b = 2 OR c = 3;",
        "WHERE triple OR"
    );
}

// MAIN
int main() {
    std::cout << "TESTS DEL PARSER\n";

    std::cout << "CREATE TABLE\n";
    testCreateTable();

    std::cout << "\nINSERT\n";
    testInsert();

    std::cout << "\nSELECT\n";
    testSelect();

    std::cout << "\nDELETE\n";
    testDelete();

    std::cout << "\nUPDATE\n";
    testUpdate();

    std::cout << "\nWHERE\n";
    testWhere();

    std::cout << "  Results: " << passed << " passed, " << failed << " failed\n";

    return failed == 0 ? 0 : 1;
}