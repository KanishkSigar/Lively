#pragma once
#include <vector>
#include <string>
#include "../ast/ast.h"
#include "tac.h"

class TACGenerator {
private:
    std::vector<TACInstruction> instructions;
    int tempCounter = 0;
    int labelCounter = 0;

public:
    std::vector<TACInstruction> generate(
        const std::vector<std::unique_ptr<Statement>>& statements);

private:
    std::string newTemp();
    std::string newLabel();

    void generateStatement(Statement* stmt);
    std::string generateExpression(Expression* expr);
};