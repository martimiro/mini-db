// Separate operations of every node

#ifndef VISITOR_H
#define VISITOR_H

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

    virtual void visit(SelectNode& node) = 0;
    virtual void visit(InsertNode& node) = 0;
    virtual void visit(CreateTableNode& node) = 0;
    virtual void visit(DeleteNode& node) = 0;
    virtual void visit(UpdateNode& node) = 0;
    virtual void visit(BinaryOpNode& node) = 0;
    virtual void visit(IdentifyNode& node) = 0;
    virtual void visit(IntLiteralNode& node) = 0;
    virtual void visit(FloatLiteralNode& node) = 0;
    virtual void visit(StringLiteralNode& node) = 0;
    virtual void visit(BoolLiteralNode& node) = 0;
    virtual void visit(CreateIndexNode& node) = 0;
    virtual void visit(BeginNode& node) = 0;
    virtual void visit(CommitNode& node) = 0;
    virtual void visit(RollbackNode& node) = 0;
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
    void visit(CreateIndexNode& node) override;
    void visit(BeginNode& node)    override;
    void visit(CommitNode& node)   override;
    void visit(RollbackNode& node) override;

  private:
    int indentification_;
    int indentificationSize_;

    // Prints actual indent
    void printIndetification() const {
      std::cout << std::string(indentification_ * indentificationSize_, ' ');
    }

    // Increments indent
    template <typename F>
    void withIndetification(F f) {
      indentification_++;
      f();
      indentification_--;
    }
};

#endif //VISITOR_H