#include "tac_generator.h"
#include <iostream>

// ==============================
// Helpers
// ==============================

std::string TACGenerator::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

std::string TACGenerator::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

// ==============================
// Entry
// ==============================

std::vector<TACInstruction> TACGenerator::generate(
    const std::vector<std::unique_ptr<Statement>>& statements) {

    instructions.clear();
    tempCounter = 0;
    labelCounter = 0;

    for (const auto& stmt : statements) {
        generateStatement(stmt.get());
    }

    return instructions;
}

// ==============================
// Statement Handling
// ==============================

void TACGenerator::generateStatement(Statement* stmt) {

    // VarDecl
    if (auto var = dynamic_cast<VarDecl*>(stmt)) {
        std::string value = generateExpression(var->value.get());
        instructions.emplace_back(TACOp::ASSIGN, var->name, value);
    }

    // Assignment
    else if (auto assign = dynamic_cast<Assignment*>(stmt)) {
        std::string value = generateExpression(assign->value.get());
        instructions.emplace_back(TACOp::ASSIGN, assign->name, value);
    }

    // Emit
    else if (auto emit = dynamic_cast<EmitStmt*>(stmt)) {
        std::string value = generateExpression(emit->value.get());
        instructions.emplace_back(TACOp::EMIT, "", value);
    }

    // If
    else if (auto ifs = dynamic_cast<IfStmt*>(stmt)) {

        std::string cond = generateExpression(ifs->condition.get());

        std::string elseLabel = newLabel();
        std::string endLabel = newLabel();

        instructions.emplace_back(TACOp::IF_FALSE, elseLabel, cond);

        for (auto& s : ifs->thenBranch)
            generateStatement(s.get());

        instructions.emplace_back(TACOp::GOTO, endLabel);

        instructions.emplace_back(TACOp::LABEL, elseLabel);

        for (auto& s : ifs->elseBranch)
            generateStatement(s.get());

        instructions.emplace_back(TACOp::LABEL, endLabel);
    }

    // Loop
    else if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {

        std::string startLabel = newLabel();
        std::string endLabel = newLabel();

        instructions.emplace_back(TACOp::LABEL, startLabel);

        std::string cond = generateExpression(loop->condition.get());
        instructions.emplace_back(TACOp::IF_FALSE, endLabel, cond);

        for (auto& s : loop->body)
            generateStatement(s.get());

        instructions.emplace_back(TACOp::GOTO, startLabel);
        instructions.emplace_back(TACOp::LABEL, endLabel);
    }

    // FunctionDecl (not yet lowered to IR — skip with warning)
    else if (dynamic_cast<FunctionDecl*>(stmt)) {
        std::cerr << "[IR Warning] FunctionDecl skipped — not yet supported in TAC.\n";
    }

    // ReturnStmt (not yet lowered to IR — skip with warning)
    else if (dynamic_cast<ReturnStmt*>(stmt)) {
        std::cerr << "[IR Warning] ReturnStmt skipped — not yet supported in TAC.\n";
    }
}

// ==============================
// Expression Handling
// ==============================

std::string TACGenerator::generateExpression(Expression* expr) {

    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        return lit->value;
    }

    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        return var->name;
    }

    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {

        std::string left = generateExpression(bin->left.get());
        std::string right = generateExpression(bin->right.get());

        std::string temp = newTemp();

        TACOp op;

        if (bin->op == "+") op = TACOp::ADD;
        else if (bin->op == "-") op = TACOp::SUB;
        else if (bin->op == "*") op = TACOp::MUL;
        else if (bin->op == "/") op = TACOp::DIV;
        else if (bin->op == ">") op = TACOp::GT;
        else if (bin->op == "<") op = TACOp::LT;
        else if (bin->op == ">=") op = TACOp::GE;
        else if (bin->op == "<=") op = TACOp::LE;
        else if (bin->op == "==") op = TACOp::EQ;
        else if (bin->op == "!=") op = TACOp::NEQ;
        else throw std::runtime_error("Unknown operator");

        instructions.emplace_back(op, temp, left, right);
        return temp;
    }

    throw std::runtime_error("Unknown expression");
}