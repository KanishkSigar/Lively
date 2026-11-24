#include "vm.h"
#include <iostream>
#include <stdexcept>
#include <cctype>

// ==============================
// Constructor
// ==============================

VM::VM(const std::vector<Instruction>& instructions)
    : instructions(instructions) {}

// ==============================
// Stack Helpers
// ==============================

void VM::push(int value) {
    stack.push_back(value);
}

int VM::pop() {
    if (stack.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    int value = stack.back();
    stack.pop_back();
    return value;
}

// ==============================
// Resolve Operand
// ==============================

std::unordered_map<std::string, int>& VM::slots(const std::string& operand) {
    if (!operand.empty() && operand.front() == '@') return frames.at(frames.size() - 1).locals;
    return memory;
}

int VM::resolveValue(const std::string& operand) {
    if (operand.empty()) return 0;

    // Check if number
    if (std::isdigit(operand[0]) || (operand[0] == '-' && operand.size() > 1 && std::isdigit(operand[1]))) {
        return std::stoi(operand);
    }

    // Otherwise variable
    auto& values = slots(operand);
    if (values.find(operand) == values.end()) {
        throw std::runtime_error("Undefined variable: " + operand);
    }

    return values.at(operand);
}

// ==============================
// Execution Loop
// ==============================

void VM::run() {

    while (ip < instructions.size()) {

        const Instruction& instr = instructions[ip];

        switch (instr.op) {

        // =========================
        // Stack / Memory
        // =========================

        case OpCode::PUSH_CONST:
            if (instr.operand == "alive") {
                push(1);
            } else if (instr.operand == "dead") {
                push(0);
            } else {
                push(std::stoi(instr.operand));
            }
            break;

        case OpCode::LOAD:
            push(resolveValue(instr.operand));
            break;

        case OpCode::STORE: {
            int value = pop();
            slots(instr.operand)[instr.operand] = value;
            break;
        }

        // =========================
        // Arithmetic
        // =========================

        case OpCode::ADD: {
            int b = pop();
            int a = pop();
            push(a + b);
            break;
        }

        case OpCode::SUB: {
            int b = pop();
            int a = pop();
            push(a - b);
            break;
        }

        case OpCode::MUL: {
            int b = pop();
            int a = pop();
            push(a * b);
            break;
        }

        case OpCode::DIV: {
            int b = pop();
            int a = pop();
            if (b == 0) throw std::runtime_error("Division by zero");
            push(a / b);
            break;
        }

        // =========================
        // Comparisons
        // =========================

        case OpCode::GT: {
            int b = pop();
            int a = pop();
            push(a > b);
            break;
        }

        case OpCode::LT: {
            int b = pop();
            int a = pop();
            push(a < b);
            break;
        }

        case OpCode::GE: {
            int b = pop();
            int a = pop();
            push(a >= b);
            break;
        }

        case OpCode::LE: {
            int b = pop();
            int a = pop();
            push(a <= b);
            break;
        }

        case OpCode::EQ: {
            int b = pop();
            int a = pop();
            push(a == b);
            break;
        }

        case OpCode::NEQ: {
            int b = pop();
            int a = pop();
            push(a != b);
            break;
        }

        // =========================
        // Control Flow
        // =========================

        case OpCode::CALL:
            if (frames.size() >= 1024) throw std::runtime_error("Call stack limit exceeded");
            frames.push_back({ip + 1, {}});
            ip = std::stoi(instr.operand);
            continue;
        case OpCode::RET:
            if (frames.empty()) throw std::runtime_error("Return outside a function");
            ip = frames.back().returnIp;
            frames.pop_back();
            continue;
        case OpCode::MISSING_RETURN:
            throw std::runtime_error("Function completed without yielding: " + instr.operand);

        case OpCode::JUMP:
            ip = std::stoi(instr.operand);
            continue;

        case OpCode::JUMP_IF_FALSE: {
            int condition = pop();
            if (!condition) {
                ip = std::stoi(instr.operand);
                continue;
            }
            break;
        }

        // =========================
        // Output
        // =========================

        case OpCode::PRINT: {
            int value = pop();
            std::cout << value << std::endl;
            break;
        }

        default:
            throw std::runtime_error("Unknown opcode");
        }

        ip++;
    }
}