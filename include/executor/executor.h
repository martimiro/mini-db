#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser/ast.h"
#include "parser/visitor.h"
#include "storage/table_manager.h"
#include "storage/record.h"
#include <string>

// Executor
// Walks the AST and executes each statement against the storage layer
class Executor : public Visitor {
    private:
        TableManager& tableManager_;

        // Evaluates a WHERE expression against a record and its schema
        bool evalWhere(ASTNode* whereNode, const Record& record, const TableSchema& schema);
        // Extract a FieldValue from a primary expression node
        FieldValue evalPrimary(ASTNode* node, const Record& record, const TableSchema& schema);
        // Compares two FieldValues from a primary expression node
        bool compareValues(const FieldValue& left, const FieldValue& right, TokenType op);
        // Find column index in schema
        int columnIndex(const TableSchema& schema, const std::string& columnName);

    public:
    explicit Executor(TableManager& tableManager) : tableManager_(tableManager) {}

        void visit(CreateTableNode &node) override;
        void visit(InsertNode &node) override;
        void visit(SelectNode &node) override;
        void visit(DeleteNode &node) override;
        void visit(UpdateNode &node) override;

        // Expression nodes
        void visit(BinaryOpNode &node) override {}
        void visit(IdentifyNode &node) override {}
        void visit(IntLiteralNode &node) override {}
        void visit(FloatLiteralNode &node) override {}
        void visit(StringLiteralNode &node) override {}
        void visit(BoolLiteralNode &node) override {}
};

#endif //EXECUTOR_H