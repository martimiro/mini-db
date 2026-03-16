#include "transaction/transaction_manager.h"
#include <stdexcept>
#include <iostream>
#include <unordered_set>

TransactionManager::TransactionManager(TableManager& tableManager,
                                        const std::string& logFile)
    : tableManager_(tableManager), wal_(logFile), nextTxnId_(1) {}

// Begin
uint32_t TransactionManager::begin() {
    uint32_t txnId = nextTxnId_++;
    transactions_[txnId] = {txnId, true, {}};

    LogRecord rec;
    rec.transactionId = txnId;
    rec.logType   = LogType::BEGIN;
    rec.tableName = "";
    rec.recordId  = {0, 0};
    wal_.append(rec);

    std::cout << "Transaction " << txnId << " started.\n";
    return txnId;
}

// Commit
void TransactionManager::commit(uint32_t txnId) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end() || !it->second.active) {
        throw std::runtime_error("Transaction " + std::to_string(txnId) + " is not active");
    }

    LogRecord rec;
    rec.transactionId = txnId;
    rec.logType   = LogType::COMMIT;
    rec.tableName = "";
    rec.recordId  = {0, 0};
    wal_.append(rec);
    wal_.flush();

    it->second.active = false;
    std::cout << "Transaction " << txnId << " committed.\n";
}

// Rollback
void TransactionManager::rollback(uint32_t txnId) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end() || !it->second.active) {
        throw std::runtime_error("Transaction " + std::to_string(txnId) + " is not active");
    }

    Transaction& txn = it->second;

    for (auto rit = txn.log.rbegin(); rit != txn.log.rend(); ++rit) {
        const LogRecord& log = *rit;

        if (log.logType == LogType::INSERT) {
            tableManager_.getHeapFile(log.tableName).deleteRecord(log.recordId);
        } else if (log.logType == LogType::DELETE) {
            tableManager_.getHeapFile(log.tableName).insertRecord(log.oldRecord);
        } else if (log.logType == LogType::UPDATE) {
            tableManager_.getHeapFile(log.tableName).deleteRecord(log.recordId);
            tableManager_.getHeapFile(log.tableName).insertRecord(log.oldRecord);
        }
    }

    LogRecord rec;
    rec.transactionId = txnId;
    rec.logType   = LogType::ROLLBACK;
    rec.tableName = "";
    rec.recordId  = {0, 0};
    wal_.append(rec);
    wal_.flush();

    txn.active = false;
    std::cout << "Transaction " << txnId << " rolled back.\n";
}

// Log operations
void TransactionManager::logInsert(uint32_t txnId, const std::string& tableName,
                                    RecordId rid, const Record& newRecord) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end() || !it->second.active) return;

    LogRecord rec;
    rec.transactionId = txnId;
    rec.logType   = LogType::INSERT;
    rec.tableName = tableName;
    rec.recordId  = rid;
    rec.newRecord = newRecord;

    wal_.append(rec);
    it->second.log.push_back(rec);
}

void TransactionManager::logDelete(uint32_t txnId, const std::string& tableName,
                                    RecordId rid, const Record& oldRecord) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end() || !it->second.active) return;

    LogRecord rec;
    rec.transactionId = txnId;
    rec.logType   = LogType::DELETE;
    rec.tableName = tableName;
    rec.recordId  = rid;
    rec.oldRecord = oldRecord;

    wal_.append(rec);
    it->second.log.push_back(rec);
}

void TransactionManager::logUpdate(uint32_t txnId, const std::string& tableName,
                                    RecordId rid, const Record& oldRecord,
                                    const Record& newRecord) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end() || !it->second.active) return;

    LogRecord rec;
    rec.transactionId = txnId;
    rec.logType   = LogType::UPDATE;
    rec.tableName = tableName;
    rec.recordId  = rid;
    rec.oldRecord = oldRecord;
    rec.newRecord = newRecord;

    wal_.append(rec);
    it->second.log.push_back(rec);
}

// Is active
bool TransactionManager::isActive(uint32_t txnId) const {
    auto it = transactions_.find(txnId);
    return it != transactions_.end() && it->second.active;
}

// Recover
void TransactionManager::recover() {
    auto logs = wal_.readAll();
    if (logs.empty()) return;

    std::cout << "[Recovery] Found " << logs.size() << " log records.\n";

    std::unordered_map<uint32_t, std::vector<LogRecord>> txnLogs;
    std::unordered_set<uint32_t> committed;
    std::unordered_set<uint32_t> rolledBack;

    for (const auto& rec : logs) {
        if (rec.logType == LogType::COMMIT)   committed.insert(rec.transactionId);
        if (rec.logType == LogType::ROLLBACK) rolledBack.insert(rec.transactionId);
        txnLogs[rec.transactionId].push_back(rec);
    }

    // Only undo incomplete transactions (BEGIN but no COMMIT or ROLLBACK)
    for (const auto& [txnId, records] : txnLogs) {
        if (committed.count(txnId) || rolledBack.count(txnId)) continue;

        std::cout << "[Recovery] Rolling back incomplete transaction "
                  << txnId << "\n";
        try {
            for (auto rit = records.rbegin(); rit != records.rend(); ++rit) {
                const LogRecord& rec = *rit;
                if (rec.logType == LogType::INSERT) {
                    tableManager_.getHeapFile(rec.tableName)
                        .deleteRecord(rec.recordId);
                } else if (rec.logType == LogType::DELETE) {
                    tableManager_.getHeapFile(rec.tableName)
                        .insertRecord(rec.oldRecord);
                } else if (rec.logType == LogType::UPDATE) {
                    tableManager_.getHeapFile(rec.tableName)
                        .deleteRecord(rec.recordId);
                    tableManager_.getHeapFile(rec.tableName)
                        .insertRecord(rec.oldRecord);
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[Recovery] Skipped transaction " << txnId
                      << " — " << e.what() << "\n";
        }
    }

    // Truncate WAL after recovery
    wal_.truncate();
}