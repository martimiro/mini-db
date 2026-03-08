// Separate operations of every node

#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include <string>
#include <iostream>

// Visotor and node reference themselves

class SelectNode;
class InsertNode;
class CreateTableNode;
class DeleteNode;
class UpdateNode;
class BinaryOpNode;
class IdentifyNode;
class IntLiteralNode;
class FloatLiteralNode;
class StringLiteralNode;
class BoolLiteralNode;

// Every operation about AST implements this interface (abstract methods)
class Visitor {
  public:
    virtual ~Visitor() = default;

    virtual void visit(SelectNode) = 0;
    virtual void visit(InsertNode) = 0;
    virtual void visit(CreateTableNode) = 0;
    virtual void visit(DeleteNode) = 0;
    virtual void visit(UpdateNode) = 0;
    virtual void visit(BinaryOpNode) = 0;
    virtual void visit(IdentifyNode) = 0;
    virtual void visit(IntLiteralNode) = 0;
    virtual void visit(FloatLiteralNode) = 0;
    virtual void visit(StringLiteralNode) = 0;
    virtual void visit(BoolLiteralNode) = 0;
};

// Print visitor -> prints AST with identitation
class PrintVisitor : public Visitor {
  public:
    explicit PrintVisitor(int indentificationSize = 2) {
      this -> indentification_ = 0;
      this -> indentificationSize_ = indentificationSize;
    }

    void visit(CreateTableNode& node) override;
    void visit(InsertNode& node) override;
    void visit(SelectNode& node) override;
    void visit(DeleteNode& node) override;
    void visit(UpdateNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(IdentifyNode& node) override;
    void visit(IntLiteralNode& node) override;
    void visit(FloatLiteralNode& node) override;
    void visit(StringLiteralNode& node) override;
    void visit(BoolLiteralNode& node) override;

  private:
    int indentification_;
    int indentificationSize_;

    // Prints actual indent
    void printIndetification() const {
      std::cout << std::string(indentification_ * indentificationSize_ + ' ') << std::endl;
    }

    // Increments indent
    void withIndetification(auto f) {
      indentification_++;
      f();
      indentification_--;
    }
};

#endif //VISITOR_H