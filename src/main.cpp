#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast_printer.h"
#include "semantic/semantic.h"
#include "ir/tac_generator.h"
#include "bytecode/bytecode_generator.h"
#include "vm/vm.h"

// Helper: Convert TokenType to a human-readable string.
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::BIND:           return "BIND";
        case TokenType::IS:             return "IS";
        case TokenType::EMIT:           return "EMIT";
        case TokenType::CHECK:          return "CHECK";
        case TokenType::OTHERWISE:      return "OTHERWISE";
        case TokenType::CYCLE:          return "CYCLE";
        case TokenType::FORGE:          return "FORGE";
        case TokenType::YIELD:          return "YIELD";

        case TokenType::TYPE_INT:       return "TYPE_INT";
        case TokenType::TYPE_BOOL:      return "TYPE_BOOL";

        case TokenType::INT_LITERAL:    return "INT_LITERAL";
        case TokenType::BOOL_LITERAL:   return "BOOL_LITERAL";

        case TokenType::IDENTIFIER:     return "IDENTIFIER";

        case TokenType::PLUS:           return "PLUS";
        case TokenType::MINUS:          return "MINUS";
        case TokenType::MULTIPLY:       return "MULTIPLY";
        case TokenType::DIVIDE:         return "DIVIDE";

        case TokenType::GREATER:        return "GREATER";
        case TokenType::LESS:           return "LESS";
        case TokenType::GREATER_EQUAL:  return "GREATER_EQUAL";
        case TokenType::LESS_EQUAL:     return "LESS_EQUAL";
        case TokenType::EQUAL:          return "EQUAL";
        case TokenType::NOT_EQUAL:      return "NOT_EQUAL";

        case TokenType::ASSIGN:         return "ASSIGN";

        case TokenType::COLON:          return "COLON";
        case TokenType::SEMICOLON:      return "SEMICOLON";
        case TokenType::COMMA:          return "COMMA";

        case TokenType::LPAREN:         return "LPAREN";
        case TokenType::RPAREN:         return "RPAREN";
        case TokenType::LBRACE:         return "LBRACE";
        case TokenType::RBRACE:         return "RBRACE";

        case TokenType::END_OF_FILE:    return "EOF";
    }

    return "UNKNOWN";
}

// Helper: Convert TACOp to a human-readable string.
std::string tacOpToString(TACOp op) {
    switch (op) {
        case TACOp::ADD:      return "ADD";
        case TACOp::SUB:      return "SUB";
        case TACOp::MUL:      return "MUL";
        case TACOp::DIV:      return "DIV";
        case TACOp::GT:       return "GT";
        case TACOp::LT:       return "LT";
        case TACOp::GE:       return "GE";
        case TACOp::LE:       return "LE";
        case TACOp::EQ:       return "EQ";
        case TACOp::NEQ:      return "NEQ";
        case TACOp::ASSIGN:   return "ASSIGN";
        case TACOp::LABEL:    return "LABEL";
        case TACOp::GOTO:     return "GOTO";
        case TACOp::IF_FALSE: return "IF_FALSE";
        case TACOp::EMIT:     return "EMIT";
    }
    return "UNKNOWN";
}

// Helper: Convert OpCode to a human-readable string.
std::string opCodeToString(OpCode op) {
    switch (op) {
        case OpCode::PUSH_CONST:    return "PUSH_CONST";
        case OpCode::LOAD:          return "LOAD";
        case OpCode::STORE:         return "STORE";
        case OpCode::ADD:           return "ADD";
        case OpCode::SUB:           return "SUB";
        case OpCode::MUL:           return "MUL";
        case OpCode::DIV:           return "DIV";
        case OpCode::GT:            return "GT";
        case OpCode::LT:            return "LT";
        case OpCode::GE:            return "GE";
        case OpCode::LE:            return "LE";
        case OpCode::EQ:            return "EQ";
        case OpCode::NEQ:           return "NEQ";
        case OpCode::JUMP:          return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::PRINT:         return "PRINT";
    }
    return "UNKNOWN";
}

// Read entire file contents into a string.
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Main — LiveLy Compiler Pipeline
// Source → Lexer → Parser (AST) → Semantic → TAC (IR) → Bytecode
int main(int argc, char* argv[]) {
    std::cout << "====================================\n";
    std::cout << "   LiveLy Compiler v0.4             \n";;
    std::cout << "====================================\n";

    if (argc < 2) {
        std::cout << "Usage: lively <source_file.lv>\n";
        return 1;
    }

    try {
        // PHASE 1: Read Source
        std::string source = readFile(argv[1]);
        std::cout << "\n[Phase 1] Source loaded: " << argv[1] << "\n";

        // PHASE 2: Lexical Analysis
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        std::cout << "[Phase 2] Lexer produced " << tokens.size() << " tokens.\n";
        std::cout << "\n--- TOKEN STREAM ---\n";

        for (const auto& token : tokens) {
            std::cout << "  " << tokenTypeToString(token.type)
                      << " (" << token.value << ") "
                      << "[Line: " << token.line
                      << ", Col: " << token.column << "]\n";
        }

        // PHASE 3: Parsing → AST
        Parser parser(tokens);
        auto ast = parser.parse();

        std::cout << "\n[Phase 3] Parser produced " << ast.size()
                  << " top-level statement" << (ast.size() != 1 ? "s" : "")
                  << ".\n";

        // PHASE 4: AST Visualization
        ASTPrinter::printProgram(ast);

        // PHASE 5: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast);

        std::cout << "\n[Phase 5] Semantic analysis passed — all types valid.\n";

        // PHASE 6: IR Generation (Three-Address Code)
        TACGenerator tacGen;
        auto tac = tacGen.generate(ast);

        std::cout << "\n[Phase 6] TAC generated — " << tac.size()
                  << " instruction" << (tac.size() != 1 ? "s" : "") << ".\n";
        std::cout << "\n--- THREE-ADDRESS CODE ---\n";
        for (const auto& instr : tac) {
            std::cout << "  " << tacOpToString(instr.op);
            if (!instr.result.empty()) std::cout << " " << instr.result;
            if (!instr.arg1.empty()) std::cout << " " << instr.arg1;
            if (!instr.arg2.empty()) std::cout << " " << instr.arg2;
            std::cout << "\n";
        }

        // PHASE 7: Bytecode Generation
        BytecodeGenerator bcGen;
        auto bytecode = bcGen.generate(tac);

        std::cout << "\n[Phase 7] Bytecode generated — " << bytecode.size()
                  << " instruction" << (bytecode.size() != 1 ? "s" : "") << ".\n";
        std::cout << "\n--- BYTECODE ---\n";
        for (size_t i = 0; i < bytecode.size(); i++) {
            std::cout << "  " << i << ": " << opCodeToString(bytecode[i].op);
            if (!bytecode[i].operand.empty())
                std::cout << " " << bytecode[i].operand;
            std::cout << "\n";
        }

        // PHASE 8: Virtual Machine Execution
        std::cout << "\n[Phase 8] Virtual machine executing bytecode...\n";
        VM vm(bytecode);
        vm.run();

        std::cout << "\n[OK] All compiler phases completed successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}