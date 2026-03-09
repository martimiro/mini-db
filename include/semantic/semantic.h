#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser/ast.h"
#include "parser/visitor.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

struct TableSchema {
    std::string name;
    std::vector<ColumnDefinition> columns;

    bool hasColumn(std::string columnName) const{
        for (auto c : columns) {
            if (c.name == columnName) {
                return true;
            }
        }
        return false;
    }
};

// Catalog
// Saves tables and their columns in memory
class Catalog {
    private:
        std::unordered_map<std::string, TableSchema> tables_;

    public:
        void addTable(const TableSchema& schema) {
            tables_[schema.name] = schema;
        }

        bool hasTable(const std::string& name) const {
            return tables_.find(name) != tables_.end();
        }

        const TableSchema& getTable(const std::string& name) const {
            return tables_.at(name);
        }
};

// Sematic error
// Typed erros for semtanic mistakes
class SemanticError : public std::runtime_error {
    public:
    explicit SemanticError(const std::string& message): std::runtime_error("SemanticError: " + message) {}
};

// Semantic analyzer
// Implements visitor
class SemanticAnalyzer : public Visitor {
    private:
        Catalog& catalog_;

        void requireTable(const std::string& tableName);
        void requireColumn(const std::string& tableName, const std::string& columnName);

    public:
        explicit SemanticAnalyzer(Catalog& catalog): catalog_(catalog) {}

        void visit(CreateTableNode& node) override;
        void visit(InsertNode& node) override;
        void visit(SelectNode& node) override;
        void visit(DeleteNode& node) override;
        void visit(UpdateNode& node) override;

        // Expression nodes
        void visit(BinaryOpNode& node) override;
        void visit(IdentifyNode& node) override {}
        void visit(IntLiteralNode& node) override {}
        void visit(FloatLiteralNode& node) override {}
        void visit(StringLiteralNode& node) override {}
        void visit(BoolLiteralNode& node) override {}
};

#endif //SEMANTIC_H