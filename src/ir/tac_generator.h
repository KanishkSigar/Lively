#pragma once
#include <vector>
#include <unordered_set>
#include <string>
#include "../ast/ast.h"
#include "tac.h"

class TACGenerator {
private:
    std::vector<TACInstruction> instructions;
    int tempCounter = 0;
    int labelCounter = 0;
    bool inFunction = false;
    std::unordered_set<std::string> localNames;
    std::string variableSlot(const std::string& name) const;

public:
    std::vector<TACInstruction> generate(
        const std::vector<std::unique_ptr<Statement>>& statements);

private:
    std::string newTemp();
    std::string newLabel();

    void generateStatement(Statement* stmt);
    std::string generateExpression(Expression* expr);
};