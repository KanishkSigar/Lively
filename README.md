# LiveLy Programming Language

LiveLy is a custom statically-typed procedural programming language with
its own compiler and stack-based virtual machine. A JIT execution engine
is planned for a future version.

## Features
- Custom statically-typed language with `int` and `bool` primitives
- Full compiler front end: lexer, parser, semantic analyzer
- Three-Address Code intermediate representation
- Stack-based bytecode with resolved jump targets
- Stack-based Virtual Machine that executes the bytecode
- JIT compilation (planned — not yet implemented)

## Current Pipeline (v0.4)

```
Source (.lv) → Lexer → Parser → AST → Semantic → TAC (IR) → Bytecode → VM
```

All phases produce diagnostic output: tokens, AST, semantic check, TAC
instructions, bytecode with resolved jump targets, and the VM's runtime
output from `emit` statements.

## Current Limitations
- `forge` functions parse and type-check but are not yet lowered to bytecode
  (no `CALL`/`RET` opcodes), so function calls do not execute at runtime.
- No strings, floats, or arrays.
- No unary or logical operators (`!`, `&&`, `||`, unary `-`).
- No source-level comments.
- JIT backend is a placeholder; only VM execution is available.

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
All 10 CTest tests (lexer, parser, semantic, TAC, bytecode, VM) run automatically.

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
| `vm_runtime` | VM correctly executes arithmetic + control flow |

## Running Your Own Programs

After building, run any `.lv` source file through the full pipeline:

```powershell
.\build\lively.exe examples\hello.lv
.\build\lively.exe examples\fibonacci.lv
.\build\lively.exe examples\loop_test.lv
```

The compiler prints diagnostics for every phase and then executes the
program via the VM. For `examples\fibonacci.lv`, the VM prints the first
12 Fibonacci numbers: `0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89`.

## VS Code Setup
Project settings enforce CMake Presets mode.
Open the folder in VS Code and run the default workflow or test from CMake Tools.