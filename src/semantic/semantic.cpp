#include <semantic/semantic.h>
#include <iostream>

// Helpers
void SemanticAnalyzer::requireTable(const std::string &tableName) {
    if (!catalog_.hasTable(tableName)) {
        throw SemanticError("Table " + tableName + " does not exist");
    }
}

void SemanticAnalyzer::requireColumn(const std::string &tableName, const std::string &columnName) {
    // * always valid
    if (columnName == "*") {
        return;
    }

    const TableSchema& schema = catalog_.getTable(tableName);
    if (!schema.hasColumn(columnName)) {
        throw SemanticError("Column " + columnName + " not found in " + tableName);
    }
}

// Visit implementations
void SemanticAnalyzer::visit(CreateTableNode& node) {
    // Table must not already exists
    if (catalog_.hasTable(node.tableName)) {
        throw SemanticError("Table " + node.tableName + " already exists");
    }

    // Column name must be unique
    std::unordered_map<std::string, bool> seen;
    for (const auto& column : node.columns) {
        if (seen.count(column.name)) {
            throw SemanticError("Column " + column.name + " already exists in " + node.tableName);
        }
        seen[column.name] = true;
    }

    TableSchema schema;
    schema.name = node.tableName;
    schema.columns = node.columns;
    catalog_.addTable(schema);

    std::cout << "Sematic table" << node.tableName<< "registred." << std::endl;
}

void SemanticAnalyzer::visit(InsertNode& node) {
    requireTable(node.tableName);
}

void SemanticAnalyzer::visit(SelectNode& node) {
    requireTable(node.tableName);

    for (const auto& column : node.columns) {
        requireColumn(node.tableName, column);
    }

    if (node.where) {
        node.where -> accept(*this);
    }
}

void SemanticAnalyzer::visit(DeleteNode& node) {
    requireTable(node.tableName);

    if (node.where) {
        node.where -> accept(*this);
    }
}

void SemanticAnalyzer::visit(UpdateNode& node) {
    requireTable(node.tableName);

    for (const auto& assignment : node.assignments) {
        requireColumn(node.tableName, assignment.column);
    }

    if (node.where) {
        node.where -> accept(*this);
    }
}

void SemanticAnalyzer::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);
}