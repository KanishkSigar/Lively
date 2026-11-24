#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "../bytecode/bytecode.h"

class VM {
private:
    std::vector<Instruction> instructions;
    std::vector<int> stack;
    std::unordered_map<std::string, int> memory;

    struct CallFrame {
        int returnIp;
        std::unordered_map<std::string, int> locals;
    };
    std::vector<CallFrame> frames;

    std::unordered_map<std::string, int>& slots(const std::string& operand);

    int ip = 0; // instruction pointer

public:
    VM(const std::vector<Instruction>& instructions);

    void run();

private:
    int pop();
    void push(int value);

    int resolveValue(const std::string& operand);
};