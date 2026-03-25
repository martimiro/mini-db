# mini-db

A relational database engine built from scratch in C++17. Implements a SQL parser, binary storage engine, B+Tree indexes, transactions with WAL, and a persistent catalog.

---

## Features

- SQL parser: SELECT, INSERT, UPDATE, DELETE, CREATE TABLE, CREATE INDEX, BEGIN, COMMIT, ROLLBACK
- Slotted page storage (4KB pages) with a buffer pool (LRU eviction)
- Record serialization to binary format
- B+Tree indexes for O(log n) lookups
- Write-Ahead Log (WAL) for transaction support
- Persistent catalog: tables survive restarts without re-running CREATE TABLE
- Interactive REPL

---

## Requirements

- C++17 or later
- CMake 3.20 or later
- A C++ compiler (GCC, Clang, or MSVC)

---

## Build

```bash
git clone https://github.com/youruser/mini-db.git
cd mini-db
mkdir build && cd build
cmake ..
cmake --build .
```

---

## Run

```bash
./mini-db
```

---

## Usage

Start the REPL and type SQL queries:

```sql
CREATE TABLE users (id INT, name TEXT, age INT);
INSERT INTO users VALUES (1, "Anna", 25);
INSERT INTO users VALUES (2, "Lluis", 30);
INSERT INTO users VALUES (3, "Maria", 22);

SELECT * FROM users;
SELECT name, age FROM users WHERE age > 24;

UPDATE users SET age = 26 WHERE id = 1;
DELETE FROM users WHERE id = 2;

CREATE INDEX idx_id ON users(id);
SELECT * FROM users WHERE id = 1;

BEGIN;
INSERT INTO users VALUES (4, "Pedro", 28);
ROLLBACK;

exit
```

Type `exit`, `quit`, or `\q` to close the REPL.

---

## SQL Reference

### CREATE TABLE

```sql
CREATE TABLE table_name (column_name TYPE, ...);
```

Supported types: INT, TEXT.

### INSERT

```sql
INSERT INTO table_name VALUES (value1, value2, ...);
```

### SELECT

```sql
SELECT * FROM table_name;
SELECT col1, col2 FROM table_name WHERE condition;
```

Supported WHERE operators: =, !=, <, >, <=, >=, AND, OR.

### UPDATE

```sql
UPDATE table_name SET col = value WHERE condition;
```

### DELETE

```sql
DELETE FROM table_name WHERE condition;
```

### CREATE INDEX

```sql
CREATE INDEX index_name ON table_name(column_name);
```

Only INT columns are supported. Once an index exists, SELECT queries with equality conditions on that column will use an index scan instead of a full table scan.

### Transactions

```sql
BEGIN;
-- one or more statements
COMMIT;

BEGIN;
-- one or more statements
ROLLBACK;
```

---

## Architecture

```
mini-db/
├── include/
│   ├── parser/          token, lexer, ast, parser, visitor
│   ├── semantic/        catalog, semantic analyzer
│   ├── storage/         page, record, disk_manager, buffer_pool, heap_file, table_manager, catalog
│   ├── index/           btree, index_manager
│   ├── executor/        executor
│   └── transaction/     wal, transaction_manager
├── src/                 implementations
└── test/                unit tests
```

### Components

**Parser** — tokenizes SQL input and builds an AST. Supports the full SELECT/INSERT/UPDATE/DELETE/CREATE TABLE/CREATE INDEX/BEGIN/COMMIT/ROLLBACK grammar.

**Storage engine** — data is stored in fixed-size 4KB pages using the slotted page format. The buffer pool caches pages in memory and evicts the least recently used page when full. The disk manager handles raw reads and writes to .db files.

**Record format** — each record is serialized to a flat byte buffer. INT fields occupy 5 bytes (1 type tag + 4 data), TEXT fields occupy 5 + N bytes (1 type tag + 4 length + N data).

**B+Tree** — in-memory B+Tree with order 4. Supports point lookups, range scans, insert, and delete. Used by CREATE INDEX to accelerate equality queries.

**Transactions** — the WAL appends log records before applying changes. On startup, the recovery process rolls back any incomplete transactions found in the log. BEGIN/COMMIT/ROLLBACK are supported.

**Persistent catalog** — table schemas are saved to catalog.dat on disk. On startup, the TableManager reads this file and reopens the corresponding HeapFiles automatically.

---

## Running Tests

Each module has its own test executable:

```bash
./test_lexer
./test_parser
./test_page
./test_disk_manager
./test_buffer_pool
./test_heap_file
./test_btree
./test_executor
```

---

## Data Directory

By default the database stores all files in `./data/` relative to the working directory:

- `./data/table_name.db` — binary page data for each table
- `./data/catalog.dat`   — persistent table schemas
- `./data/wal.log`       — write-ahead log

To reset the database completely:

```bash
rm -rf ./data
```

---
