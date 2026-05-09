#include "bytecode_generator.h"
#include <cctype>

// ==============================
// Helpers
// ==============================

bool BytecodeGenerator::isNumber(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start >= s.size()) return false;
    for (size_t i = start; i < s.size(); i++) {
        if (!std::isdigit(s[i])) return false;
    }
    return true;
}

void BytecodeGenerator::emitLoad(const std::string& operand) {
    if (isNumber(operand) || operand == "alive" || operand == "dead") {
        instructions.emplace_back(OpCode::PUSH_CONST, operand);
    } else {
        instructions.emplace_back(OpCode::LOAD, operand);
    }
}

int BytecodeGenerator::bytecodeCountForTAC(TACOp op) {
    switch (op) {
        case TACOp::ASSIGN:  return 2;   // emitLoad + STORE
        case TACOp::ADD: case TACOp::SUB: case TACOp::MUL: case TACOp::DIV:
        case TACOp::GT:  case TACOp::LT:  case TACOp::GE:  case TACOp::LE:
        case TACOp::EQ:  case TACOp::NEQ:
            return 4;   // emitLoad + emitLoad + OP + STORE
        case TACOp::IF_FALSE: return 2;  // emitLoad + JUMP_IF_FALSE
        case TACOp::GOTO:     return 1;  // JUMP
        case TACOp::EMIT:     return 2;  // emitLoad + PRINT
        case TACOp::LABEL:    return 0;
        default:              return 0;
    }
}

// ==============================
// Entry
// ==============================

std::vector<Instruction> BytecodeGenerator::generate(
    const std::vector<TACInstruction>& tac) {

    instructions.clear();
    labelMap.clear();

    firstPass(tac);
    secondPass(tac);
    return instructions;
}

// ==============================
// Pass 1: Resolve Labels
// ==============================

void BytecodeGenerator::firstPass(const std::vector<TACInstruction>& tac) {
    int index = 0;

    for (const auto& instr : tac) {
        if (instr.op == TACOp::LABEL) {
            labelMap[instr.result] = index;
        } else {
            index += bytecodeCountForTAC(instr.op);
        }
    }
}

// ==============================
// Pass 2: Generate Bytecode
// ==============================

void BytecodeGenerator::secondPass(const std::vector<TACInstruction>& tac) {

    for (const auto& instr : tac) {

        switch (instr.op) {

        case TACOp::ASSIGN:
            emitLoad(instr.arg1);
            instructions.emplace_back(OpCode::STORE, instr.result);
            break;

        case TACOp::ADD:
        case TACOp::SUB:
        case TACOp::MUL:
        case TACOp::DIV:
        case TACOp::GT:
        case TACOp::LT:
        case TACOp::GE:
        case TACOp::LE:
        case TACOp::EQ:
        case TACOp::NEQ:

            emitLoad(instr.arg1);
            emitLoad(instr.arg2);

            if      (instr.op == TACOp::ADD) instructions.emplace_back(OpCode::ADD);
            else if (instr.op == TACOp::SUB) instructions.emplace_back(OpCode::SUB);
            else if (instr.op == TACOp::MUL) instructions.emplace_back(OpCode::MUL);
            else if (instr.op == TACOp::DIV) instructions.emplace_back(OpCode::DIV);
            else if (instr.op == TACOp::GT)  instructions.emplace_back(OpCode::GT);
            else if (instr.op == TACOp::LT)  instructions.emplace_back(OpCode::LT);
            else if (instr.op == TACOp::GE)  instructions.emplace_back(OpCode::GE);
            else if (instr.op == TACOp::LE)  instructions.emplace_back(OpCode::LE);
            else if (instr.op == TACOp::EQ)  instructions.emplace_back(OpCode::EQ);
            else if (instr.op == TACOp::NEQ) instructions.emplace_back(OpCode::NEQ);

            instructions.emplace_back(OpCode::STORE, instr.result);
            break;

        case TACOp::IF_FALSE:
            emitLoad(instr.arg1);
            instructions.emplace_back(OpCode::JUMP_IF_FALSE,
                std::to_string(labelMap[instr.result]));
            break;

        case TACOp::GOTO:
            instructions.emplace_back(OpCode::JUMP,
                std::to_string(labelMap[instr.result]));
            break;

        case TACOp::EMIT:
            emitLoad(instr.arg1);
            instructions.emplace_back(OpCode::PRINT);
            break;

        default:
            break;
        }
    }
}