#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "vm/vm.h"

namespace {

std::string runVmAndCaptureOutput(const std::vector<Instruction>& instructions) {
    VM vm(instructions);

    std::ostringstream captured;
    std::streambuf* originalBuffer = std::cout.rdbuf(captured.rdbuf());

    try {
        vm.run();
    } catch (...) {
        std::cout.rdbuf(originalBuffer);
        throw;
    }

    std::cout.rdbuf(originalBuffer);
    return captured.str();
}

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        const std::vector<Instruction> program = {
            Instruction(OpCode::PUSH_CONST, "2"),
            Instruction(OpCode::PUSH_CONST, "3"),
            Instruction(OpCode::ADD),
            Instruction(OpCode::STORE, "tmp"),
            Instruction(OpCode::LOAD, "tmp"),
            Instruction(OpCode::PUSH_CONST, "4"),
            Instruction(OpCode::MUL),
            Instruction(OpCode::PRINT),
            Instruction(OpCode::PUSH_CONST, "0"),
            Instruction(OpCode::JUMP_IF_FALSE, "12"),
            Instruction(OpCode::PUSH_CONST, "99"),
            Instruction(OpCode::PRINT),
            Instruction(OpCode::PUSH_CONST, "7"),
            Instruction(OpCode::PRINT)
        };

        const std::string output = runVmAndCaptureOutput(program);
        require(output == "20\n7\n", "Unexpected VM output: " + output);

        std::cout << "VM runtime tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "VM runtime tests failed: " << e.what() << '\n';
        return 1;
    }
}