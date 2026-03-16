#include "parser/ast.h"
#include "parser/visitor.h"

#include <iostream>

// Implementation PrintVisitor
void PrintVisitor::visit(CreateTableNode& node) {
    printIndetification();
    std::cout<< "Create TableNode ("<< node.tableName << ")"<<std::endl;
    withIndetification([&]() {
        for (const auto& column : node.columns) {
            std::cout<< "Column: "<< column.name << ": " << tokenTypeName(column.type) << std::endl;
        }
    });
}

void PrintVisitor::visit(InsertNode &node) {
    printIndetification();
    std::cout<< "Insert Node (" << node.tableName << ")"<<std::endl;
    withIndetification([&]() {
       for (const auto& value : node.values) {
           value->accept(*this);
       }
    });
}

void PrintVisitor::visit(SelectNode &node) {
    printIndetification();
    std::cout<< "Select"<<std::endl;
    withIndetification([&]() {
        // Columns
        printIndetification();
        std::cout<< "Columns: ";
        for (size_t i = 0; i < node.columns.size(); i++) {
            std::cout << node.columns[i];
            if (i < node.columns.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;

        // Table
        printIndetification();
        std::cout<< "From: " << node.tableName << std::endl;

        if (node.where) {
            printIndetification();
            std::cout<< "Where"<<std::endl;
            withIndetification([&]() {
                node.where->accept(*this);
            });
        }
    });
}

void PrintVisitor::visit(DeleteNode &node) {
    printIndetification();
    std::cout<< "Delete Node (" << node.tableName << ")"<<std::endl;
    withIndetification([&]() {
       if (node.where) {
           printIndetification();
           std::cout<< "Where"<<std::endl;
           withIndetification([&]() {
               node.where->accept(*this);
           });
       }
    });
}

void PrintVisitor::visit(UpdateNode &node) {
    printIndetification();
    std::cout<< "Update Node (" << node.tableName << ")"<<std::endl;
    withIndetification([&]() {
        for (const auto& assignment: node.assignments) {
            printIndetification();
            std::cout<< "Set :"<< assignment.column << " =" << std::endl;

            withIndetification([&]() {
                assignment.value->accept(*this);
            });
        }
        if (node.where) {
            printIndetification();
            std::cout<< "Where"<<std::endl;
            withIndetification([&]() {
                node.where->accept(*this);
            });
        }
    });
}

void PrintVisitor::visit(BinaryOpNode &node) {
    printIndetification();
    std::cout << "BinaryOpNode (" << tokenTypeName(node.op) << ")"<<std::endl;
    withIndetification([&]() {
        node.left->accept(*this);
        node.right->accept(*this);
    });
}

void PrintVisitor::visit(IdentifyNode &node) {
    printIndetification();
    std::cout<< "Identifier (" << node.name << ")"<<std::endl;
}

void PrintVisitor::visit(IntLiteralNode &node) {
    printIndetification();
    std::cout<< "IntLiteral (" << node.value << ")"<<std::endl;
}

void PrintVisitor::visit(FloatLiteralNode &node) {
    printIndetification();
    std::cout<< "FloatLiteral (" << node.value << ")"<<std::endl;
}

void PrintVisitor::visit(StringLiteralNode &node) {
    printIndetification();
    std::cout<< "StringLiteral (" << node.value << ")"<<std::endl;
}

void PrintVisitor::visit(BoolLiteralNode &node) {
    printIndetification();
    std::cout<< "BoolLiteral (" << node.value << ")"<<std::endl;
}

// Implementation of accept
void IdentifyNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void IntLiteralNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void FloatLiteralNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void StringLiteralNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void BoolLiteralNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void BinaryOpNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void CreateTableNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void InsertNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void SelectNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void DeleteNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void UpdateNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void PrintVisitor::visit(CreateIndexNode& node) {
    printIndetification();
    std::cout<< "CreateIndexNode (" << node.tableName << ")"<<std::endl;
}

void CreateIndexNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void PrintVisitor::visit(BeginNode& node) {
    printIndetification();
    std::cout << "Begin\n";
}

void PrintVisitor::visit(CommitNode& node) {
    printIndetification();
    std::cout << "Commit\n";
}

void PrintVisitor::visit(RollbackNode& node) {
    printIndetification();
    std::cout << "Rollback\n";
}

void BeginNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void CommitNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void RollbackNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}