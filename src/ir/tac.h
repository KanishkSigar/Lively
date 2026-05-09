#pragma once
#include <string>

enum class TACOp {
    ADD, SUB, MUL, DIV,
    GT, LT, GE, LE,
    EQ, NEQ,
    ASSIGN,

    LABEL,
    GOTO,
    IF_FALSE,

    EMIT
};

struct TACInstruction {
    TACOp op;
    std::string result;
    std::string arg1;
    std::string arg2;

    TACInstruction(TACOp op,
                   std::string result = "",
                   std::string arg1 = "",
                   std::string arg2 = "")
        : op(op), result(result), arg1(arg1), arg2(arg2) {}
};