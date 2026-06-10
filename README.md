# LiveLy Programming Language

> A custom statically-typed procedural language with its own compiler and stack-based virtual machine — built from scratch in C++.

[![C++](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.25+-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)
[![Tests](https://img.shields.io/badge/CTest-10%20passing-2ea44f)](#test-suite)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

LiveLy is a custom statically-typed procedural programming language with
its own compiler and stack-based virtual machine. Source code runs through a
complete front end (lexer → parser → semantic analysis), lowers to a
Three-Address Code IR, compiles to bytecode, and executes on a custom VM.
A JIT execution engine is planned for a future version.

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

### Language at a glance

LiveLy uses its own keywords rather than the usual C-family ones:

| LiveLy | Meaning | Equivalent |
|---|---|---|
| `bind x:int is 10;` | Typed variable declaration + assignment | `int x = 10;` |
| `x is x - 1;` | Reassignment | `x = x - 1;` |
| `check (...) { } otherwise { }` | Conditional | `if / else` |
| `cycle (...) { }` | Loop | `while` |
| `emit x;` | Print to output | `print(x)` |
| `forge` | Function definition (parses; not yet executable) | `function` |

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

If `cmake` is not on your PATH (common right after a fresh winget install,
because already-open terminals cache the old environment), load the dev
environment first:

```cmd
:: in cmd
dev

:: in PowerShell — note the leading dot (dot-source so PATH persists)
. .\dev.ps1
```

`dev.bat` / `dev.ps1` prepend the CMake, Ninja, and WinLibs paths to the
current session's PATH and warn about any missing tool.

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

## License

Released under the [MIT License](LICENSE).

## Acknowledgments

Developed with the assistance of AI-powered coding tools (Claude Code) for parts of the implementation and documentation.