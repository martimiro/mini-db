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

        // Compare INT
        switch (op) {
            case TokenType::TOKEN_OPERATOR_EQUAL:
                return l == r;
            case TokenType::TOKEN_OPERATOR_NOT_EQUAL:
                return l != r;
            case TokenType::TOKEN_OPERATOR_GREATER_THAN:
                return l > r;
            case TokenType::TOKEN_OPERATOR_GREATER_THAN_OR_EQUAL:
                return l >= r;
            case TokenType::TOKEN_OPERATOR_LESS_THAN:
                return l < r;
            case TokenType::TOKEN_OPERATOR_LESS_THAN_OR_EQUAL:
                return l <= r;
            default:
                throw std::invalid_argument("No such token type");
        }
    }

    // Compare TEXT
    if (left.type == FieldType::TEXT && right.type == FieldType::TEXT) {
        switch (op) {
            case TokenType::TOKEN_OPERATOR_EQUAL:
                return left.textVale == right.textVale;
            case TokenType::TOKEN_OPERATOR_NOT_EQUAL:
                return left.textVale != right.textVale;
            default:
                throw std::invalid_argument("No such token type");
        }
    }

    throw std::invalid_argument("No such token type");
}

bool Executor::evalWhere(ASTNode *whereNode, const Record &record, const TableSchema &schema) {
    if (!whereNode) {
        return true;
    }

    if (auto* bin = dynamic_cast<BinaryOpNode*> (whereNode)) {
        if (bin->op == TokenType::TOKEN_OPERATOR_AND) {
            return evalWhere(bin->left.get(), record, schema) && evalWhere(bin->right.get(), record, schema);
        }

        if (bin->op == TokenType::TOKEN_OPERATOR_OR) {
            return evalWhere(bin->left.get(), record, schema) || evalWhere(bin->right.get(), record, schema);
        }

        // Comparaisom
        FieldValue left = evalPrimary(bin->left.get(), record, schema);
        FieldValue right = evalPrimary(bin->right.get(), record, schema);
        return compareValues(left, right, bin->op) ;
    }

    throw std::invalid_argument("No such token type");
}


// Create Table
void Executor::visit(CreateTableNode& node) {
    tableManager_.createTable(node.tableName, node.columns);
    std::cout << node.tableName << " created." <<std::endl;
}

// Insert
void Executor::visit(InsertNode& node) {
    const TableSchema &schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);

    if (node.values.size() != schema.columns.size()) {
        throw std::invalid_argument("Not enough columns in table");
    }

    Record record;
    for (size_t i = 0; i < node.values.size(); i++) {
        ASTNode* valueNode = node.values[i].get();
        TokenType columnType = schema.columns[i].type;

        if (columnType == TokenType::TOKEN_KEYWORD_INT) {
            if (auto* n = dynamic_cast<IntLiteralNode *>(valueNode)) {
                record.fieldValues.push_back(FieldValue::makeInt(n->value));
            } else {
                throw std::invalid_argument("No such token type");
            }
        } else if (columnType == TokenType::TOKEN_KEYWORD_TEXT) {
            if (auto* n = dynamic_cast<StringLiteralNode *>(valueNode)) {
                record.fieldValues.push_back(FieldValue::makeText(n->value));
            } else {
                throw std::invalid_argument("No such token type");
            }
        }
    }

    RecordId recordId = hf.insertRecord(record);
    std::cout << "Insert at page " << recordId.pageId << ", slot " << recordId.slotId << "." <<std::endl;
}

// Select
void Executor::visit(SelectNode& node) {
    const TableSchema &schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);
    bool selectAll = (node.columns.size() == 1) && node.columns[0] == "*";

    // Header
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

    int count = 0;
    for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
        Page* page = hf.getBufferPool().fetchPage(pageId);
        uint32_t numSlots = page->getNumSlots();
        hf.getBufferPool().unpinPage(pageId, false);

        for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
            try {
                Record record = hf.getRecord({pageId, slotId});

                if (!evalWhere(node.where.get(), record, schema)) continue;

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
                        int index = columnIndex(schema, node.columns[i]);
                        if (record.fieldValues[index].type == FieldType::INT) {
                            std::cout << record.fieldValues[index].intValue;
                        } else {
                            std::cout << record.fieldValues[index].textVale;
                        }
                        if (i < node.columns.size() - 1) std::cout << " | ";
                    }
                }
                std::cout << "\n";
                count++;
            } catch (const std::exception&) {
                // Deleted slot — skip
            }
        }
    }
    std::cout << "(" << count << " rows)\n";
}

// Delete
void Executor::visit(DeleteNode& node) {
    const TableSchema &schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);

    int count = 0;
    for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
        Page* page = hf.getBufferPool().fetchPage(pageId);
        uint32_t numSlots = page->getNumSlots();
        hf.getBufferPool().unpinPage(pageId, false);

        for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
            try {
                Record record = hf.getRecord({pageId, slotId});
                if (evalWhere(node.where.get(), record, schema)) {
                    hf.deleteRecord({pageId, slotId});
                    count++;
                }
            } catch (const std::exception&) {
            }
        }
    }
    std::cout << "(" << count << " rows deleted)\n";
}

// Update
void Executor::visit(UpdateNode& node) {
    const TableSchema &schema = tableManager_.getTableSchema(node.tableName);
    HeapFile& hf = tableManager_.getHeapFile(node.tableName);

    int count = 0;
    for (uint32_t pageId = 0; pageId < hf.numPages(); pageId++) {
        Page* page = hf.getBufferPool().fetchPage(pageId);
        uint32_t numSlots = page->getNumSlots();
        hf.getBufferPool().unpinPage(pageId, false);

        for (uint32_t slotId = 0; slotId < numSlots; slotId++) {
            try {
                Record record = hf.getRecord({pageId, slotId});
                if (!evalWhere(node.where.get(), record, schema)) {
                    continue;
                }

                for (const auto& assingment : node.assignments) {
                    int index = columnIndex(schema, assingment.column);
                    ASTNode* valueNode = assingment.value.get();

                    if (auto* n = dynamic_cast<IntLiteralNode *>(valueNode)) {
                        record.fieldValues[index] = FieldValue::makeInt(n->value);
                    } else if (auto* s = dynamic_cast<StringLiteralNode *>(valueNode)) {
                        record.fieldValues[index] = FieldValue::makeText(s->value);
                    }
                }

                hf.deleteRecord({pageId, slotId});
                hf.insertRecord(record);
                count++;
            } catch (std::exception&) {

            }
        }
    }

    std::cout << count << " row(s) updated.\n";
}