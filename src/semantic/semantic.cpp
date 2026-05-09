#include "semantic.h"
#include <stdexcept>
#include <iostream>

// Constructor — initialise with a global scope
SemanticAnalyzer::SemanticAnalyzer()
    : currentReturnType("") {
    pushScope(); // global scope
}

// ===================== Scope Management =====================

void SemanticAnalyzer::pushScope() {
    scopes.emplace_back();
}

void SemanticAnalyzer::popScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void SemanticAnalyzer::declare(const std::string& name,
                               const std::string& type) {
    auto& current = scopes.back();
    if (current.find(name) != current.end()) {
        error("Variable already declared in this scope: " + name);
    }
    current[name] = type;
}

std::string SemanticAnalyzer::lookup(const std::string& name) {
    // Walk scopes from innermost to outermost
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return it->second;
        }
    }
    error("Undefined variable: " + name);
    return ""; // unreachable
}

// ===================== Entry Point ==========================

void SemanticAnalyzer::analyze(
    const std::vector<std::unique_ptr<Statement>>& statements) {

    for (const auto& stmt : statements) {
        analyzeStatement(stmt.get());
    }
}

// ===================== Statement Analysis ===================

void SemanticAnalyzer::analyzeStatement(Statement* stmt) {

    // Variable Declaration: bind x:int is 10;
    if (auto var = dynamic_cast<VarDecl*>(stmt)) {

        std::string valueType = analyzeExpression(var->value.get());

        if (valueType != var->type) {
            error("Type mismatch in declaration of '" + var->name
                  + "': expected " + var->type + ", got " + valueType);
        }

        declare(var->name, var->type);
    }

    // Assignment: x is expr;
    else if (auto assign = dynamic_cast<Assignment*>(stmt)) {

        std::string varType = lookup(assign->name);
        std::string valueType = analyzeExpression(assign->value.get());

        if (varType != valueType) {
            error("Type mismatch in assignment of '" + assign->name
                  + "': expected " + varType + ", got " + valueType);
        }
    }

    // Emit: emit expr;
    else if (auto emit = dynamic_cast<EmitStmt*>(stmt)) {
        analyzeExpression(emit->value.get());
    }

    // If: check (cond) { ... } otherwise { ... }
    else if (auto ifs = dynamic_cast<IfStmt*>(stmt)) {

        std::string condType = analyzeExpression(ifs->condition.get());
        if (condType != "bool") {
            error("Condition in 'check' must be bool, got " + condType);
        }

        for (const auto& s : ifs->thenBranch)
            analyzeStatement(s.get());

        for (const auto& s : ifs->elseBranch)
            analyzeStatement(s.get());
    }

    // Loop: cycle (cond) { ... }
    else if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {

        std::string condType = analyzeExpression(loop->condition.get());
        if (condType != "bool") {
            error("Condition in 'cycle' must be bool, got " + condType);
        }

        for (const auto& s : loop->body)
            analyzeStatement(s.get());
    }

    // Return: yield expr;
    else if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {

        std::string valueType = analyzeExpression(ret->value.get());

        if (!currentReturnType.empty() && valueType != currentReturnType) {
            error("Return type mismatch: expected " + currentReturnType
                  + ", got " + valueType);
        }
    }

    // Function Declaration: forge name(params): returnType { body }
    else if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {

        // Register the function name in the current scope
        // (stored as type "func" for now — future: full signature)
        declare(fn->name, "func");

        // Create a new scope for the function body
        pushScope();

        // Register parameters in the function's scope
        for (const auto& param : fn->parameters) {
            declare(param.first, param.second);
        }

        // Track return type for yield-checking
        std::string previousReturnType = currentReturnType;
        currentReturnType = fn->returnType;

        // Analyze the function body
        for (const auto& s : fn->body) {
            analyzeStatement(s.get());
        }

        // Restore previous context
        currentReturnType = previousReturnType;
        popScope();
    }
}

// ===================== Expression Analysis ==================

std::string SemanticAnalyzer::analyzeExpression(Expression* expr) {

    // Literal: 42, alive, dead
    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        if (lit->value == "alive" || lit->value == "dead") {
            return "bool";
        }
        return "int";
    }

    // Variable: x, y, name
    else if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        return lookup(var->name);
    }

    // Binary Expression: a + b, x >= y, a == b
    else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {

        std::string left  = analyzeExpression(bin->left.get());
        std::string right = analyzeExpression(bin->right.get());
        std::string op    = bin->op;

        // Arithmetic: +  -  *  /
        if (op == "+" || op == "-" || op == "*" || op == "/") {
            if (left != "int" || right != "int") {
                error("Arithmetic operator '" + op
                      + "' requires int operands, got " + left + " and " + right);
            }
            return "int";
        }

        // Comparison: >  <  >=  <=
        if (op == ">" || op == "<" || op == ">=" || op == "<=") {
            if (left != "int" || right != "int") {
                error("Comparison operator '" + op
                      + "' requires int operands, got " + left + " and " + right);
            }
            return "bool";
        }

        // Equality: ==  !=
        if (op == "==" || op == "!=") {
            if (left != right) {
                error("Equality operator '" + op
                      + "' requires matching types, got " + left + " and " + right);
            }
            return "bool";
        }

        error("Unknown operator: " + op);
    }

    error("Unknown expression type");
    return ""; // unreachable
}

// ===================== Error Handler ========================

void SemanticAnalyzer::error(const std::string& message) {
    throw std::runtime_error("Semantic Error: " + message);
}