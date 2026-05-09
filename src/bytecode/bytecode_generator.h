#pragma once
#include <vector>
#include <unordered_map>
#include "../ir/tac.h"
#include "bytecode.h"

class BytecodeGenerator {
private:
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, int> labelMap;

public:
    std::vector<Instruction> generate(const std::vector<TACInstruction>& tac);

private:
    void firstPass(const std::vector<TACInstruction>& tac);
    void secondPass(const std::vector<TACInstruction>& tac);

    bool isNumber(const std::string& s);
    void emitLoad(const std::string& operand);
    int bytecodeCountForTAC(TACOp op);
};