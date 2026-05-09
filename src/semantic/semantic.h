#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "../ast/ast.h"

class SemanticAnalyzer {
private:
    // Scope stack — each scope is a map of variable name → type.
    // Index 0 is global scope; new scopes are pushed for functions/blocks.
    std::vector<std::unordered_map<std::string, std::string>> scopes;

    // Track the expected return type when inside a function body.
    // Empty string means "not inside any function".
    std::string currentReturnType;

public:
    SemanticAnalyzer();

    void analyze(const std::vector<std::unique_ptr<Statement>>& statements);

private:
    // Scope management
    void pushScope();
    void popScope();
    void declare(const std::string& name, const std::string& type);
    std::string lookup(const std::string& name);

    // Analysis
    void analyzeStatement(Statement* stmt);
    std::string analyzeExpression(Expression* expr);

    void error(const std::string& message);
};