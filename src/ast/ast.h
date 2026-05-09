#pragma once
#include <string>
#include <vector>
#include <memory>

// Base AST Node
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Expressions
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Literal Expression (int / bool)
class LiteralExpr : public Expression {
public:
    std::string value;

    LiteralExpr(const std::string& value)
        : value(value) {}
};

// Variable Expression
class VariableExpr : public Expression {
public:
    std::string name;

    VariableExpr(const std::string& name)
        : name(name) {}
};

// Binary Expression (a + b, x > y)
class BinaryExpr : public Expression {
public:
    std::string op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryExpr(std::string op,
               std::unique_ptr<Expression> left,
               std::unique_ptr<Expression> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
};

// Statements
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Variable Declaration
class VarDecl : public Statement {
public:
    std::string name;
    std::string type;
    std::unique_ptr<Expression> value;

    VarDecl(std::string name,
            std::string type,
            std::unique_ptr<Expression> value)
        : name(name), type(type), value(std::move(value)) {}
};

// Assignment
class Assignment : public Statement {
public:
    std::string name;
    std::unique_ptr<Expression> value;

    Assignment(std::string name,
               std::unique_ptr<Expression> value)
        : name(name), value(std::move(value)) {}
};

// Emit Statement
class EmitStmt : public Statement {
public:
    std::unique_ptr<Expression> value;

    EmitStmt(std::unique_ptr<Expression> value)
        : value(std::move(value)) {}
};

// If Statement
class IfStmt : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    std::vector<std::unique_ptr<Statement>> elseBranch;

    IfStmt(std::unique_ptr<Expression> condition)
        : condition(std::move(condition)) {}
};

// Loop Statement
class LoopStmt : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;

    LoopStmt(std::unique_ptr<Expression> condition)
        : condition(std::move(condition)) {}
};

// Return Statement
class ReturnStmt : public Statement {
public:
    std::unique_ptr<Expression> value;

    ReturnStmt(std::unique_ptr<Expression> value)
        : value(std::move(value)) {}
};

// Function Declaration
class FunctionDecl : public Statement {
public:
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::string returnType;
    std::vector<std::unique_ptr<Statement>> body;

    FunctionDecl(std::string name, std::string returnType)
        : name(name), returnType(returnType) {}
};