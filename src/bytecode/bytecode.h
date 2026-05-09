#pragma once
#include <string>

enum class OpCode {
    PUSH_CONST,
    LOAD,
    STORE,

    ADD, SUB, MUL, DIV,
    GT, LT, GE, LE,
    EQ, NEQ,

    JUMP,
    JUMP_IF_FALSE,

    PRINT
};

struct Instruction {
    OpCode op;
    std::string operand;

    Instruction(OpCode op, std::string operand = "")
        : op(op), operand(operand) {}
};