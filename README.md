# LiveLy

LiveLy is a statically typed procedural language with a compiler pipeline that lowers source code into TAC, bytecode, and runtime execution machinery. The repository contains the lexer, parser, semantic analyzer, IR generation, bytecode generator, VM, and related tests.

## Highlights
- Source language with explicit typing and structured control flow
- Compiler pipeline: lexer -> parser -> AST -> semantic analysis -> TAC -> bytecode
- Diagnostics across the compilation stages for easier debugging
- Example programs and negative test cases under `examples/`

## Quick Start

### Requirements
- CMake 3.25+
- Ninja 1.11+
- A C++17-capable compiler such as GCC 11+, Clang 14+, or MSVC 19.3+

### Build and test
From the project root, run:

```powershell
cmake --workflow --preset ci
```

That single command configures the project, builds it, and runs the full CTest suite.

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

## Test Coverage

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

## Repository Layout

- `src/` compiler, IR, VM, and support code
- `tests/` lexer and VM tests
- `examples/` sample LiveLy programs
- `docs/` design notes and language details

## VS Code

The project is configured for CMake Presets mode. Open the folder in VS Code and use CMake Tools to build or run the workflow preset.