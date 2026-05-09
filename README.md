# LiveLy Programming Language

LiveLy is a custom statically-typed procedural programming language 
with its own compiler, virtual machine, and JIT execution engine.

## Features
- Custom language design
- Compiler pipeline (Lexer → Parser → Semantic → TAC → Bytecode)
- Three-Address Code intermediate representation
- Stack-based bytecode generation
- Stack-based Virtual Machine (planned)
- JIT compilation (planned)

## Current Pipeline (v0.4)

```
Source (.lv) → Lexer → Parser → AST → Semantic → TAC (IR) → Bytecode
```

All phases produce diagnostic output showing tokens, AST, TAC instructions,
and bytecode with resolved jump targets.

## Example
```lively
bind x:int is 10;
bind y:int is 2 + 3 * (4 - 1);

check (x >= 5) {
    emit x;
} otherwise {
    emit 0;
}

cycle (x != 0) {
    x is x - 1;
}
```

## Tooling Requirements
- CMake: 3.25+
- Ninja: 1.11+
- C++ compiler with C++17 support (GCC 11+, Clang 14+, or MSVC 19.3+)
- VS Code extension (recommended): CMake Tools (ms-vscode.cmake-tools), latest stable

## Single Command Build + Test
From project root, run:

```powershell
cmake --workflow --preset ci
```

This command performs configure + build + test in one step.
All 9 CTest tests (lexer, parser, semantic, TAC, bytecode) run automatically.

## Test Suite

| Test | Validates |
|------|-----------|
| `lexer_hello` | Token stream output |
| `parser_hello` | AST generation |
| `semantic_hello` | Type-safe code passes analysis |
| `semantic_type_mismatch` | Type error is rejected |
| `semantic_undefined` | Undeclared variable is rejected |
| `tac_hello` | TAC contains ASSIGN + EMIT |
| `tac_loop` | TAC control flow (LABEL, IF_FALSE, GOTO) |
| `bytecode_hello` | Bytecode has PUSH_CONST, STORE, PRINT |
| `bytecode_loop` | Bytecode jump instructions |

## VS Code Setup
Project settings enforce CMake Presets mode.
Open the folder in VS Code and run the default workflow or test from CMake Tools.