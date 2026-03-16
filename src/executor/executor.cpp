#include "executor/executor.h"
#include <stdexcept>
#include <iostream>

// Helpers
int Executor::columnIndex(const TableSchema &schema, const std::string &columnName) {
    for (size_t i = 0; i < schema.columns.size(); i++) {
        if (schema.columns[i].name == columnName) {
            return static_cast<int>(i);
        }
    }
    throw std::invalid_argument("No such column: " + columnName);
}

FieldValue Executor::evalPrimary(ASTNode *node, const Record &record, const TableSchema &schema) {
    if (auto* id = dynamic_cast<IdentifyNode *>(node)) {
        int index = columnIndex(schema, id->name);
        return record.fieldValues[index];
    }
    if (auto* n = dynamic_cast<IntLiteralNode *>(node)) {
        return FieldValue::makeInt(n->value);
    }
    if (auto* s = dynamic_cast<StringLiteralNode *>(node)) {
        return FieldValue::makeText(s->value);
    }
    if (auto* b = dynamic_cast<BoolLiteralNode *>(node)) {
        return FieldValue::makeInt(b->value ? 1 : 0);
    }
    if (auto* t = dynamic_cast<FloatLiteralNode *>(node)) {
        return FieldValue::makeInt(static_cast<int32_t>(t->value));
    }
    throw std::runtime_error("Executor: Cannot evaluate expression");
}

bool Executor::compareValues(const FieldValue &left, const FieldValue &right, TokenType op) {
    if (left.type == FieldType::INT && right.type == FieldType::INT) {
        int64_t l = left.intValue;
        int64_t r = right.intValue;
        switch (op) {
            case TokenType::TOKEN_OPERATOR_EQUAL:                return l == r;
            case TokenType::TOKEN_OPERATOR_NOT_EQUAL:            return l != r;
            case TokenType::TOKEN_OPERATOR_GREATER_THAN:         return l > r;
            case TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL:return l >= r;
            case TokenType::TOKEN_OPERATOR_LESS_THAN:            return l < r;
            case TokenType::TOKEN_OPERATOR_LESS_THAN_OR_EQUAL:   return l <= r;
            default: throw std::invalid_argument("Unsupported operator");
        }
    }
    if (left.type == FieldType::TEXT && right.type == FieldType::TEXT) {
        switch (op) {
            case TokenType::TOKEN_OPERATOR_EQUAL:    return left.textVale == right.textVale;
            case TokenType::TOKEN_OPERATOR_NOT_EQUAL:return left.textVale != right.textVale;
            default: throw std::invalid_argument("Unsupported operator for TEXT");
        }
    }
    throw std::invalid_argument("Type mismatch in comparison");
}

bool Executor::evalWhere(ASTNode *whereNode, const Record &record, const TableSchema &schema) {
    if (!whereNode) return true;

    if (auto* bin = dynamic_cast<BinaryOpNode*>(whereNode)) {
        if (bin->op == TokenType::TOKEN_OPERATOR_AND) {
            return evalWhere(bin->left.get(), record, schema) &&
                   evalWhere(bin->right.get(), record, schema);
        }
        if (bin->op == TokenType::TOKEN_OPERATOR_OR) {
            return evalWhere(bin->left.get(), record, schema) ||
                   evalWhere(bin->right.get(), record, schema);
        }
        FieldValue left  = evalPrimary(bin->left.get(),  record, schema);
        FieldValue right = evalPrimary(bin->right.get(), record, schema);
        return compareValues(left, right, bin->op);
    }
    throw std::invalid_argument("Unexpected WHERE node");
}

// Create Table
void Executor::visit(CreateTableNode& node) {
    tableManager_.createTable(node.tableName, node.columns);
    std::cout << node.tableName << " created.\n";
}

// Insert
void Executor::visit(InsertNode& node) {
    const TableSchema& schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);

    if (node.values.size() != schema.columns.size()) {
        throw std::invalid_argument("Not enough columns in table");
    }

    Record record;
    for (size_t i = 0; i < node.values.size(); i++) {
        ASTNode* valueNode = node.values[i].get();
        TokenType columnType = schema.columns[i].type;

        if (columnType == TokenType::TOKEN_KEYWORD_INT) {
            if (auto* n = dynamic_cast<IntLiteralNode*>(valueNode)) {
                record.fieldValues.push_back(FieldValue::makeInt(n->value));
            } else {
                throw std::invalid_argument("Expected INT value");
            }
        } else if (columnType == TokenType::TOKEN_KEYWORD_TEXT) {
            if (auto* n = dynamic_cast<StringLiteralNode*>(valueNode)) {
                record.fieldValues.push_back(FieldValue::makeText(n->value));
            } else {
                throw std::invalid_argument("Expected TEXT value");
            }
        }
    }

    RecordId rid = hf.insertRecord(record);

    // Log for transaction
    if (currentTxnId_ != 0 && txnManager_.isActive(currentTxnId_)) {
        txnManager_.logInsert(currentTxnId_, node.tableName, rid, record);
    }

    // Update indexes
    IndexManager& im = tableManager_.getIndexManager();
    for (size_t i = 0; i < schema.columns.size(); i++) {
        if (im.hasIndex(node.tableName, schema.columns[i].name)) {
            BTreeKey key = record.fieldValues[i].intValue;
            im.insertKey(node.tableName, schema.columns[i].name, key, rid);
        }
    }

    std::cout << "Inserted at page " << rid.pageId
              << ", slot " << rid.slotId << ".\n";
}

// Select
void Executor::visit(SelectNode& node) {
    const TableSchema& schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);
    IndexManager& im = tableManager_.getIndexManager();
    bool selectAll = (node.columns.size() == 1) && node.columns[0] == "*";

    // Print header
    if (selectAll) {
        for (size_t i = 0; i < schema.columns.size(); i++) {
            std::cout << schema.columns[i].name;
            if (i < schema.columns.size() - 1) std::cout << " | ";
        }
    } else {
        for (size_t i = 0; i < node.columns.size(); i++) {
            std::cout << node.columns[i];
            if (i < node.columns.size() - 1) std::cout << " | ";
        }
    }
    std::cout << "\n" << std::string(40, '-') << "\n";

    // Check for index scan
    std::vector<RecordId> candidates;
    bool usedIndex = false;

    if (node.where) {
        if (auto* bin = dynamic_cast<BinaryOpNode*>(node.where.get())) {
            if (bin->op == TokenType::TOKEN_OPERATOR_EQUAL) {
                if (auto* id = dynamic_cast<IdentifyNode*>(bin->left.get())) {
                    if (auto* val = dynamic_cast<IntLiteralNode*>(bin->right.get())) {
                        if (im.hasIndex(node.tableName, id->name)) {
                            auto rid = im.getIndex(node.tableName, id->name)
                                         .search(val->value);
                            if (rid.has_value()) candidates.push_back(*rid);
                            usedIndex = true;
                        }
                    }
                }
            }
        }
    }

    int count = 0;

    if (usedIndex) {
        for (const auto& rid : candidates) {
            try {
                Record record = hf.getRecord(rid);
                if (!evalWhere(node.where.get(), record, schema)) continue;
                printRow(record, node, schema, selectAll);
                count++;
            } catch (const std::exception&) {}
        }
    } else {
        for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
            Page* page = hf.getBufferPool().fetchPage(pageId);
            uint32_t numSlots = page->getNumSlots();
            hf.getBufferPool().unpinPage(pageId, false);

            for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
                try {
                    Record record = hf.getRecord({pageId, slotId});
                    if (!evalWhere(node.where.get(), record, schema)) continue;
                    printRow(record, node, schema, selectAll);
                    count++;
                } catch (const std::exception&) {}
            }
        }
    }

    std::cout << "(" << count << " rows)\n";
    std::cout << (usedIndex ? "[index scan]\n" : "[full scan]\n");
}

// Delete
void Executor::visit(DeleteNode& node) {
    const TableSchema& schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);
    IndexManager& im = tableManager_.getIndexManager();

    int count = 0;
    for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
        Page* page = hf.getBufferPool().fetchPage(pageId);
        uint32_t numSlots = page->getNumSlots();

        for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
            try {
                Record record = hf.getRecord({pageId, slotId});
                if (!evalWhere(node.where.get(), record, schema)) continue;

                // Log for transaction
                if (currentTxnId_ != 0 && txnManager_.isActive(currentTxnId_)) {
                    txnManager_.logDelete(currentTxnId_, node.tableName,
                                          {pageId, slotId}, record);
                }

                // Remove from indexes
                for (size_t i = 0; i < schema.columns.size(); i++) {
                    if (im.hasIndex(node.tableName, schema.columns[i].name)) {
                        BTreeKey key = record.fieldValues[i].intValue;
                        im.removeKey(node.tableName, schema.columns[i].name, key);
                    }
                }

                hf.deleteRecord({pageId, slotId});
                count++;
            } catch (const std::exception&) {}
        }

        hf.getBufferPool().unpinPage(pageId, false);
    }
    std::cout << "(" << count << " rows deleted)\n";
}

// Uptade
void Executor::visit(UpdateNode& node) {
    const TableSchema& schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);
    IndexManager& im = tableManager_.getIndexManager();

    int count = 0;
    for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
        Page* page = hf.getBufferPool().fetchPage(pageId);
        uint32_t numSlots = page->getNumSlots();

        for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
            try {
                Record oldRecord = hf.getRecord({pageId, slotId});
                if (!evalWhere(node.where.get(), oldRecord, schema)) continue;

                // Remove old keys from indexes
                for (size_t i = 0; i < schema.columns.size(); i++) {
                    if (im.hasIndex(node.tableName, schema.columns[i].name)) {
                        BTreeKey key = oldRecord.fieldValues[i].intValue;
                        im.removeKey(node.tableName, schema.columns[i].name, key);
                    }
                }

                // Apply assignments
                Record newRecord = oldRecord;
                for (const auto& assignment : node.assignments) {
                    int idx = columnIndex(schema, assignment.column);
                    ASTNode* valueNode = assignment.value.get();
                    if (auto* n = dynamic_cast<IntLiteralNode*>(valueNode)) {
                        newRecord.fieldValues[idx] = FieldValue::makeInt(n->value);
                    } else if (auto* s = dynamic_cast<StringLiteralNode*>(valueNode)) {
                        newRecord.fieldValues[idx] = FieldValue::makeText(s->value);
                    }
                }

                // Log for transaction
                if (currentTxnId_ != 0 && txnManager_.isActive(currentTxnId_)) {
                    txnManager_.logUpdate(currentTxnId_, node.tableName,
                                          {pageId, slotId}, oldRecord, newRecord);
                }

                hf.deleteRecord({pageId, slotId});
                RecordId newRid = hf.insertRecord(newRecord);

                // Insert new keys into indexes
                for (size_t i = 0; i < schema.columns.size(); i++) {
                    if (im.hasIndex(node.tableName, schema.columns[i].name)) {
                        BTreeKey key = newRecord.fieldValues[i].intValue;
                        im.insertKey(node.tableName, schema.columns[i].name, key, newRid);
                    }
                }

                count++;
            } catch (const std::exception&) {}
        }

        hf.getBufferPool().unpinPage(pageId, false);
    }
    std::cout << count << " row(s) updated.\n";
}

// Create index
void Executor::visit(CreateIndexNode& node) {
    const TableSchema& schema = tableManager_.getTableSchema(node.tableName);

    int colIdx = -1;
    for (size_t i = 0; i < schema.columns.size(); i++) {
        if (schema.columns[i].name == node.columnName) {
            colIdx = static_cast<int>(i);
            break;
        }
    }

    if (colIdx == -1) {
        throw std::runtime_error("Column '" + node.columnName +
                                 "' not found in table '" + node.tableName + "'");
    }

    tableManager_.getIndexManager().createIndex(
        node.tableName, node.columnName, node.indexName, colIdx);

    HeapFile& hf = tableManager_.getHeapFile(node.tableName);
    int count = 0;

    for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
        Page* page = hf.getBufferPool().fetchPage(pageId);
        uint32_t numSlots = page->getNumSlots();

        for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
            try {
                Record record = hf.getRecord({pageId, slotId});
                BTreeKey key = record.fieldValues[colIdx].intValue;
                tableManager_.getIndexManager().insertKey(
                    node.tableName, node.columnName, key, {pageId, slotId});
                count++;
            } catch (const std::exception&) {}
        }

        hf.getBufferPool().unpinPage(pageId, false);
    }

    std::cout << "Index '" << node.indexName << "' created on "
              << node.tableName << "(" << node.columnName << ")"
              << " — " << count << " entries indexed.\n";
}

// Transactions
void Executor::visit(BeginNode& node) {
    if (currentTxnId_ != 0 && txnManager_.isActive(currentTxnId_)) {
        throw std::runtime_error("Transaction already active");
    }
    currentTxnId_ = txnManager_.begin();
}

void Executor::visit(CommitNode& node) {
    if (currentTxnId_ == 0 || !txnManager_.isActive(currentTxnId_)) {
        throw std::runtime_error("No active transaction");
    }
    txnManager_.commit(currentTxnId_);
    currentTxnId_ = 0;
}

void Executor::visit(RollbackNode& node) {
    if (currentTxnId_ == 0 || !txnManager_.isActive(currentTxnId_)) {
        throw std::runtime_error("No active transaction");
    }
    txnManager_.rollback(currentTxnId_);
    currentTxnId_ = 0;
}

// Print row
void Executor::printRow(const Record& record, const SelectNode& node,
                         const TableSchema& schema, bool selectAll) {
    if (selectAll) {
        for (size_t i = 0; i < record.fieldValues.size(); i++) {
            if (record.fieldValues[i].type == FieldType::INT) {
                std::cout << record.fieldValues[i].intValue;
            } else {
                std::cout << record.fieldValues[i].textVale;
            }
            if (i < record.fieldValues.size() - 1) std::cout << " | ";
        }
    } else {
        for (size_t i = 0; i < node.columns.size(); i++) {
            int idx = columnIndex(schema, node.columns[i]);
            if (record.fieldValues[idx].type == FieldType::INT) {
                std::cout << record.fieldValues[idx].intValue;
            } else {
                std::cout << record.fieldValues[idx].textVale;
            }
            if (i < node.columns.size() - 1) std::cout << " | ";
        }
    }
    std::cout << "\n";
}