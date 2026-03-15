#include "executor/executor.h"
#include "storage/table_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include <iostream>
#include <filesystem>

int passed = 0;
int failed = 0;


// Utils
void assertTrue(bool condition, const std::string& testName) {
    if (condition) {
        std::cout << "PASS [" << testName << "]\n";
        passed++;
    } else {
        std::cout << "FAIL [" << testName << "]\n";
        failed++;
    }
}

template<typename F>
void assertThrows(F f, const std::string& testName) {
    try {
        f();
        std::cout << "FAIL [" << testName << "] — expected exception\n";
        failed++;
    } catch (const std::exception&) {
        std::cout << "PASS [" << testName << "] (exception expected)\n";
        passed++;
    }
}

void removeDir(const std::string& path) {
    std::filesystem::remove_all(path);
}

bool execQuery(Executor& executor, const std::string& sql) {
    try {
        Lexer lexer(sql);
        auto tokens = lexer.tokenizeAll();
        Parser parser(std::move(tokens));
        auto ast = parser.parse();
        ast->accept(executor);
        return true;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << "\n";
        return false;
    }
}

// Test
void testCreateTable() {
    const std::string dir = "/tmp/test_exec_create";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    assertTrue(execQuery(exec, "CREATE TABLE t (id INT, nombre TEXT);"),
               "CREATE TABLE executes without error");
    assertTrue(tm.hasTable("t"),
               "Table registered in TableManager");

    removeDir(dir);
}

void testCreateTableDuplicate() {
    const std::string dir = "/tmp/test_exec_dup";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE t (id INT);");
    assertTrue(!execQuery(exec, "CREATE TABLE t (id INT);"),
               "Duplicate CREATE TABLE throws error");

    removeDir(dir);
}

void testInsert() {
    const std::string dir = "/tmp/test_exec_insert";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE users (id INT, name TEXT);");
    assertTrue(execQuery(exec, "INSERT INTO users VALUES (1, \"Ana\");"),
               "INSERT executes without error");

    removeDir(dir);
}

void testInsertWrongColumnCount() {
    const std::string dir = "/tmp/test_exec_insert_wrong";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE t (id INT, name TEXT);");
    assertTrue(!execQuery(exec, "INSERT INTO t VALUES (1);"),
               "INSERT with wrong column count throws error");

    removeDir(dir);
}

void testSelectAll() {
    const std::string dir = "/tmp/test_exec_select";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE t (id INT, name TEXT);");
    execQuery(exec, "INSERT INTO t VALUES (1, \"Ana\");");
    execQuery(exec, "INSERT INTO t VALUES (2, \"Luis\");");

    assertTrue(execQuery(exec, "SELECT * FROM t;"),
               "SELECT * executes without error");

    removeDir(dir);
}

void testSelectWithWhere() {
    const std::string dir = "/tmp/test_exec_where";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE t (id INT, name TEXT, age INT);");
    execQuery(exec, "INSERT INTO t VALUES (1, \"Ana\", 25);");
    execQuery(exec, "INSERT INTO t VALUES (2, \"Luis\", 30);");
    execQuery(exec, "INSERT INTO t VALUES (3, \"Maria\", 22);");

    assertTrue(execQuery(exec, "SELECT * FROM t WHERE age > 24;"),
               "SELECT with WHERE executes without error");

    removeDir(dir);
}

void testDelete() {
    const std::string dir = "/tmp/test_exec_delete";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE t (id INT, name TEXT);");
    execQuery(exec, "INSERT INTO t VALUES (1, \"Ana\");");
    execQuery(exec, "INSERT INTO t VALUES (2, \"Luis\");");

    assertTrue(execQuery(exec, "DELETE FROM t WHERE id = 1;"),
               "DELETE executes without error");

    HeapFile& hf = tm.getHeapFile("t");
    bool deleted = false;
    try {
        hf.getRecord({0, 0});
    } catch (const std::exception&) {
        deleted = true;
    }
    assertTrue(deleted, "Record deleted correctly");

    removeDir(dir);
}

void testUpdate() {
    const std::string dir = "/tmp/test_exec_update";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    execQuery(exec, "CREATE TABLE t (id INT, name TEXT, age INT);");
    execQuery(exec, "INSERT INTO t VALUES (1, \"Ana\", 25);");

    assertTrue(execQuery(exec, "UPDATE t SET age = 30 WHERE id = 1;"),
               "UPDATE executes without error");

    // Verify updated value
    HeapFile& hf = tm.getHeapFile("t");
    bool updated = false;
    try {
        // After update: delete+insert, new record is at slot 1
        Record r = hf.getRecord({0, 1});
        updated = (r.fieldValues[2].intValue == 30);
    } catch (...) {}
    assertTrue(updated, "Record updated correctly");

    removeDir(dir);
}

void testSelectUnknownTable() {
    const std::string dir = "/tmp/test_exec_unknown";
    removeDir(dir);

    TableManager tm(dir);
    Executor exec(tm);

    assertTrue(!execQuery(exec, "SELECT * FROM noexiste;"),
               "SELECT on unknown table throws error");

    removeDir(dir);
}

void testPersistence() {
    const std::string dir = "/tmp/test_exec_persist";
    removeDir(dir);

    {
        TableManager tm(dir);
        Executor exec(tm);
        execQuery(exec, "CREATE TABLE t (id INT, name TEXT);");
        execQuery(exec, "INSERT INTO t VALUES (1, \"Ana\");");
        execQuery(exec, "INSERT INTO t VALUES (2, \"Luis\");");
    }

    {
        TableManager tm(dir);
        Executor exec(tm);
        execQuery(exec, "CREATE TABLE t (id INT, name TEXT);");

        HeapFile& hf = tm.getHeapFile("t");
        bool found = false;
        try {
            Record r = hf.getRecord({0, 0});
            found = (r.fieldValues[0].intValue == 1);
        } catch (...) {}
        assertTrue(found, "Data persists after reopening");
    }

    removeDir(dir);
}

// Main
int main() {
    std::cout << "TESTS EXECUTOR\n\n";

    std::cout << "CREATE TABLE\n";
    testCreateTable();
    testCreateTableDuplicate();

    std::cout << "\nINSERT\n";
    testInsert();
    testInsertWrongColumnCount();

    std::cout << "\nSELECT\n";
    testSelectAll();
    testSelectWithWhere();

    std::cout << "\nDELETE\n";
    testDelete();

    std::cout << "\nUPDATE\n";
    testUpdate();

    std::cout << "\nERRORS\n";
    testSelectUnknownTable();

    std::cout << "\nPERSISTENCE\n";
    testPersistence();

    std::cout << "\nResults: " << passed << " passed, "
              << failed << " failed\n";

    return failed == 0 ? 0 : 1;
}