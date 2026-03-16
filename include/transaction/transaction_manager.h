#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "transaction/wal.h"
#include "storage/table_manager.h"
#include <unordered_map>
#include <vector>


// Transaction
// Tracks operations in a single transaction
struct Transaction {
    uint32_t txnId;
    bool     active;
    std::vector<LogRecord> log;  // in-memory log for rollback
};

// Transaction Manager
// Manages BEGIN / COMMIT / ROLLBACK
class TransactionManager {
public:
    TransactionManager(TableManager& tableManager, const std::string& logFile);
    // Start a new transaction — returns txnId
    uint32_t begin();
    // Commit a transaction
    void commit(uint32_t txnId);
    // Rollback a transaction
    void rollback(uint32_t txnId);
    // Log an INSERT operation
    void logInsert(uint32_t txnId, const std::string& tableName, RecordId rid, const Record& newRecord);
    // Log a DELETE operation
    void logDelete(uint32_t txnId, const std::string& tableName, RecordId rid, const Record& oldRecord);
    // Log an UPDATE operation
    void logUpdate(uint32_t txnId, const std::string& tableName, RecordId rid, const Record& oldRecord,
        const Record& newRecord);
    // Check if a transaction is active
    bool isActive(uint32_t txnId) const;
    // Recover from WAL on startup
    void recover();

private:
    TableManager& tableManager_;
    WAL           wal_;
    uint32_t      nextTxnId_;
    std::unordered_map<uint32_t, Transaction> transactions_;
};

#endif // TRANSACTION_MANAGER_H