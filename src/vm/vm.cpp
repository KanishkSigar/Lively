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

int VM::resolveValue(const std::string& operand) {
    if (operand.empty()) return 0;

    // Check if number
    if (std::isdigit(operand[0]) || (operand[0] == '-' && operand.size() > 1 && std::isdigit(operand[1]))) {
        return std::stoi(operand);
    }

    // Otherwise variable
    if (memory.find(operand) == memory.end()) {
        throw std::runtime_error("Undefined variable: " + operand);
    }

    return memory[operand];
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
            memory[instr.operand] = value;
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