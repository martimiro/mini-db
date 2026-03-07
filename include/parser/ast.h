// ast.h
#ifndef AST_H
#define AST_H

#include "token.h"
#include <string>
#include <vector>
#include <memory>   // For smart pointers

class ASTNode;
using ASTNodePointer = std::unique_ptr<ASTNode>;

class ASTNode {
    protected:
        // Generete spaces
        std::string indentitationString (int indentitation) const {
            return std::string(indentitation * 2, ' ');
        }

    public:
        // Destructor (virtual for herency)
        virtual ~ASTNode() = default;
        // ONLY FOR DEBUG
        // Abstract method (every subclass must implement)
        virtual std::string toString(int indentitation = 0) const = 0;
};

// Name of a column
class IdentifyNode : public ASTNode {
    public:
        std::string name;   // Name of the column

        // Constructor
        explicit IdentifyNode(std::string name) {
            this->name = std::move(name);
        }

        std::string toString(int indentitation = 0) const override {
            return indentitationString(indentitation) + "Indetifier (" + name + ")";
        }
};

// EXPRESSION NODES

// Represents a int
class IntLiteralNode : public ASTNode {
    public:
        int value;

        // Constructor
        explicit IntLiteralNode(int v) {
            this-> value = v;
        }

        std::string toString(int indentitation = 0) const override {
            return indentitationString(indentitation) + "IntLiteral (" + std::to_string(value) + ")";
        }
};

class FloatLiteralNode : public ASTNode {
    public:
        float value;

        // Constructor
        explicit FloatLiteralNode(float v) {
            this->value = v;
        }

        std::string toString(int indentitation = 0) const override {
            return indentitationString(indentitation) + "FloatLiteral (" + std::to_string(value) + ")";
        }
};

class BoolLiteralNode : public ASTNode {
    public:
        bool value;

        // Constructor
        explicit BoolLiteralNode(bool v) {
            this->value = v;
        }

        std::string toString(int indentitation = 0) const override {
            return indentitationString(indentitation) + "BoolLiteral (" + std::to_string(value) + ")";
        }
};

class BinaryOpNode : public ASTNode {
    public:
        ASTNodePointer left;    // Left
        ASTNodePointer right;   // Right
        TokenType op;           // Operator: GREATER_THAN, OR, AND ...

        BinaryOpNode(ASTNodePointer left, ASTNodePointer right, TokenType op) {
            this->left = std::move(left);
            this->right = std::move(right);
            this->op = op;
        }

        std::string toString(int indentitation = 0) const override {
            return indentitationString(indentitation) + "BinaryOP (\n" +
                left -> toString(indentitation + 1) + "\n" +
                indentitationString(indentitation + 1) + tokenTypeName(op) + "\n" +
                right -> toString(indentitation + 1) + "\n" +
                indentitationString(indentitation) + ")";
        }
};

// SENTENCE NODES

// Represents identificacion of a column (id INT or name TEXT)
struct ColumnDefinition {
    std::string name;
    TokenType type;
};

// CREATE TABLE (id INT, name TEXY, age INT)
class CreateTableNode : public ASTNode {
    public:
        std::string tableName;  // users
        std::vector<ColumnDefinition> columns;  // [{id, INT}, {name, TEXT}, {age, INT}]

        std::string toString(int indentitation = 0) const override {
            std::string str = (indentitation) + "CreateTable (" + tableName + ")\n";
            for (const auto& column : columns ) {
                str += indentitationString(indentitation + 1) + "ColumnDefinition (" + column.name + ")" + ", " +
                    tokenTypeName(column.type) + ")\n";
            }

            return str;
        }
};

// Insert node
// INSERT INTO users VALUES (1, "John", 25);
class InsertNode : public ASTNode {
    public:
        std::string tableName;  // Users
        std::vector<ASTNodePointer> values; // [IntLiteral(1), StringLiteral(John), IntLiteral(25)]

        std::string toString(int indentitation = 0) const override {
            std::string str = indentitationString(indentitation) + "InsertNode (" + tableName + ")\n";
            for (const auto& value : values) {
                str += value->toString(indentitation + 1) + "\n";
            }

            return str;
        }
};

// SelectNode
// SELECT name
// age FROM users WHERE age > 20;
class SelectNode : public ASTNode {
    public:
        std::string tableName;  // ["name", "age"] or ["*"]
        std::vector<std::string> columns;    // Users
        ASTNodePointer where;   // If not WHERE: nullptr or if WHERE: BinaryOpNode

        std::string toString(int indentitation = 0) const override {
            std::string str = indentitationString(indentitation) + "SelectNode \n";
            str += indentitationString(indentitation + 1) + "Columns: ";
            for (size_t i = 0; i < columns.size(); i++) {
                str += columns[i];
                if (i < columns.size() - 1) {
                    str += ", ";
                }
            }
            str += "\n";
            str += indentitationString(indentitation + 1) + "From: " + tableName + ")\n";
            if (where) {
                str += indentitationString(indentitation + 1) + "Where:\n";
                str += where->toString(indentitation + 2) + "\n";
            }
            return str;
        }
};

// Delete Node
// DELETE FROM usuarios WHERE id = 1;
class DeleteNode : public ASTNode {
    public:
        std::string tableName;
        ASTNodePointer where;

        std::string toString(int indentitation = 0) const override {
            std::string str = indentitationString(indentitation) + "DeleteNode (" + tableName + ")\n";
            if (where) {
                str += indentitationString(indentitation + 1) + "Where:\n";
                str += where->toString(indentitation + 2) + "\n";
            }
            return str;
        }
};

// UpdateNode
// UPDATE users SET name = "Mary", age = 30 WHERE id = 1;
class UpdateNode : public ASTNode {
    public:
        // name_column = "name"
        // Example: name = "John" -> Assingment {"name", StringLiteralNode("John")}
        struct Assignment {
            std::string column;
            ASTNodePointer value;
        };

        std::string tableName;
        ASTNodePointer where;
        std::vector<Assignment> assignments;

        std::string toString(int indentitation = 0) const override {
            std::string str = indentitationString(indentitation) + "Update (" + tableName + ")\n";
            for (const auto& assignment : assignments) {
                str += indentitationString(indentitation + 1) + "Set " + assignment.column + " = \n" ;
                str += assignment.value ->toString(indentitation + 2) + "\n";
            }

            if (where) {
                str += indentitationString(indentitation + 1) + "Where:\n";
                str += where->toString(indentitation + 2) + "\n";
            }

            return str;
        }
};

#endif // AST_H