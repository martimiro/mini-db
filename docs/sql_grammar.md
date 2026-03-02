# SQL Grammar Documentation

## 1. Overview

This document defines the **SQL grammar** supported by our system and the corresponding **tokens**. It describes the syntax rules for valid SQL statements, including **DDL, DML, DCL, and TCL**, and the components of SQL expressions such as identifiers, literals, operators, and delimiters.

SQL statements are structured sequences of **tokens**. A **token** is the smallest meaningful unit in SQL, such as a keyword (`SELECT`), identifier (`employee`), literal (`42`), operator (`=`), or delimiter (`;`).

---

## 2. SQL Statement Categories

### 2.1 DDL – Data Definition Language

DDL statements define database structures like tables, columns, and indexes.

**Supported Statements:**

#### CREATE TABLE

CREATE TABLE <table_name> (
<column_definition> [, <column_definition> ...]
);

- `<column_definition>`: `<column_name> <data_type> [constraint]`
- `data_type` examples: `INT`, `VARCHAR(n)`, `DATE`, `FLOAT`
- `constraint` examples: `PRIMARY KEY`, `NOT NULL`, `UNIQUE`, `FOREIGN KEY`

#### ALTER TABLE

ALTER TABLE <table_name> ADD COLUMN <column_definition>;
ALTER TABLE <table_name> DROP COLUMN <column_name>;

#### DROP TABLE

DROP TABLE <table_name>;

---

### 2.2 DML – Data Manipulation Language

DML statements manipulate data inside tables.

#### SELECT

SELECT <column_list>
FROM <table> [JOIN <table> ON <condition> ...]
[WHERE <condition>]
[GROUP BY <column_list>]
[HAVING <condition>]
[ORDER BY <column_list> [ASC|DESC]]
[LIMIT <n>];

- `<column_list>`: `*` or comma-separated identifiers
- `<condition>`: comparison, logical, or membership expressions
- Joins: `INNER`, `LEFT`, `RIGHT`, `FULL`

#### INSERT

INSERT INTO <table> [(<column_list>)]
VALUES (<value_list>);

- `<value_list>` must match column types
- Optionally: `INSERT ... SELECT ...`

#### UPDATE

UPDATE <table>
SET <column1>=<value1>, <column2>=<value2>, ...
[WHERE <condition>];

#### DELETE

DELETE FROM <table>
[WHERE <condition>];

---

### 2.3 DCL – Data Control Language

DCL statements manage permissions.

GRANT <privilege> ON <table> TO <user>;
REVOKE <privilege> ON <table> FROM <user>;

- `<privilege>`: `SELECT`, `INSERT`, `UPDATE`, `DELETE`
- `<user>`: database user or role

---

### 2.4 TCL – Transaction Control Language

TCL statements control transactions.

BEGIN TRANSACTION;
COMMIT;
ROLLBACK;


---

## 3. SQL Grammar Components

### 3.1 Identifiers
- Names of tables, columns, databases, or users
- Pattern: starts with a letter or `_`, followed by letters, digits, or `_`
- Case-sensitive or insensitive depending on the database

### 3.2 Literals
- **Numeric**: integers (`123`), floats (`3.14`), negative numbers (`-42`)
- **String**: enclosed in single quotes (`'text'`)
- **Boolean**: `TRUE`, `FALSE`
- **NULL**: special null value

### 3.3 Operators
- Comparison: `=`, `!=`, `<>`, `<`, `>`, `<=`, `>=`
- Logical: `AND`, `OR`, `NOT`
- Arithmetic: `+`, `-`, `*`, `/`
- Membership/Pattern: `LIKE`, `IN`, `BETWEEN`, `IS NULL`

### 3.4 Delimiters
- Comma `,` separates elements
- Semicolon `;` ends statements
- Parentheses `(`, `)` group expressions
- Dot `.` used for table.column references

### 3.5 Keywords
- Reserved words: `SELECT`, `FROM`, `WHERE`, `JOIN`, `ON`, `GROUP BY`, `ORDER BY`, `HAVING`, `LIMIT`, `INSERT`, `UPDATE`, `DELETE`, `CREATE`, `DROP`, `ALTER`, `GRANT`, `REVOKE`, `BEGIN`, `COMMIT`, `ROLLBACK`, `TRUE`, `FALSE`, `NULL`

---

## 4. Tokens Definition

Each token is a recognized lexical unit in SQL. Tokens are defined in `token.h`.

| Token Type | Description | Examples |
|------------|------------|----------|
| KEYWORD | Reserved SQL words | `SELECT`, `INSERT`, `UPDATE`, `DELETE`, `CREATE` |
| IDENTIFIER | Names of tables, columns, users | `employee`, `salary`, `user_123` |
| LITERAL | Constants | `42`, `'John'`, `3.14`, `TRUE`, `NULL` |
| OPERATOR | Comparison, arithmetic, logical | `=`, `!=`, `<`, `>`, `+`, `-`, `AND`, `OR` |
| DELIMITER | Syntax separators | `,`, `;`, `(`, `)` |
| DOT | Member access | `.` (used as `table.column`) |

**Example tokenization:**

SQL: SELECT name, age FROM employees WHERE age > 30;
Tokens:
SELECT -> KEYWORD

name -> IDENTIFIER

, -> DELIMITER

age -> IDENTIFIER

FROM -> KEYWORD

employees-> IDENTIFIER

WHERE -> KEYWORD

age -> IDENTIFIER

-> OPERATOR

30 -> LITERAL

; -> DELIMITER


---

## 5. Notes

- The **lexer** converts raw SQL text into tokens.
- The **parser** checks if tokens form a valid SQL statement according to this grammar.
- Reserved keywords cannot be used as identifiers unless quoted (depends on SQL dialect).
- This grammar supports standard ANSI SQL statements; extensions (like `UPSERT` or `MERGE`) can be added later.