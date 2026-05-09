# LiveLy Compiler — Architecture Overview

## 1. System Architecture

The LiveLy compiler is structured as a **classic multi-phase compiler** with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────────────────┐
│                           LiveLy Compiler v0.4                      │
└─────────────────────────────────────────────────────────────────────┘

   FRONTEND (Validation)        MIDDLE-END (Lowering)   BACKEND (Execution)
  ┌─────────────────────┐      ┌──────────────────┐    ┌──────────────┐
  │ Lexical Analysis    │      │ IR Generation    │    │ VM Execution │
  │ (Tokenization)      │  →   │ (TAC Generation) │ →  │ (Bytecode)   │
  │                     │      │                  │    │              │
  ├─────────────────────┤      ├──────────────────┤    ├──────────────┤
  │ Token Stream        │      │ Three-Address    │    │ Operand      │
  │                     │      │ Code (Linear IR) │    │ Stack + Mem  │
  └─────────────────────┘      └──────────────────┘    └──────────────┘
           ↓                           ↓                        ↓
  ┌─────────────────────┐      ┌──────────────────┐
  │ Parsing             │      │ Bytecode Gen     │
  │ (Syntax → AST)      │  →   │ (TAC → Stack IR) │
  │                     │      │                  │
  └─────────────────────┘      └──────────────────┘
           ↓
  ┌─────────────────────┐
  │ Semantic Analysis   │
  │ (Type + Scope)      │  ✓ Validated syntax & semantics
  │                     │
  └─────────────────────┘
```

## 2. Phase Overview

### Phase 1: Lexer (Tokenization)

**Input:** Raw source code string
**Output:** Linear token stream

```
"bind x:int is 10;" 
    ↓
[BIND, IDENTIFIER("x"), COLON, TYPE_INT, ASSIGN, INT_LITERAL("10"), SEMICOLON, EOF]
```

**Purpose:** Decompose text into the smallest meaningful units.

### Phase 2: Parser (Syntax Analysis)

**Input:** Token stream
**Output:** Abstract Syntax Tree (AST)

```
Token stream
    ↓
IDENTIFIER(x) ← TYPE_INT ← IS ← COLON ← x ← BIND
    ↓
VarDecl("x", "int", LiteralExpr("10"))
```

**Purpose:** Enforce grammar rules and build a tree representation.

### Phase 3: AST (Data Structure)

**Input:** Parser output
**Output:** In-memory tree

```
Class hierarchy:
  Expression → LiteralExpr, VariableExpr, BinaryExpr
  Statement → VarDecl, Assignment, IfStmt, LoopStmt, ...
```

**Purpose:** Provide a structured, walkable representation of the program.

### Phase 4: Semantic Analysis

**Input:** AST
**Output:** Validated AST + Symbol table

```
"bind x:int is alive;"
    ↓ (semantic check)
Type mismatch: expected int, got bool ✗
```

**Purpose:** Validate types, scope, and name resolution.

### Phase 5: IR Generation (Three-Address Code)

**Input:** Validated AST
**Output:** Flat, linear TAC instruction list

```
VarDecl(x=10) → ASSIGN x 10
BinaryExpr(4-1) → SUB t0 4 1
LoopStmt(x<10) → LABEL L0; IF_FALSE L1 cond; ... GOTO L0
```

**Purpose:** Lower tree structure into executable form.

### Phase 6: Bytecode Generation

**Input:** TAC instructions
**Output:** Stack-based bytecode

```
ASSIGN x 10     →  PUSH_CONST 10; STORE x
ADD t0 a b      →  LOAD a; LOAD b; ADD; STORE t0
LABEL L0        →  (resolved to bytecode index)
JUMP L0         →  JUMP <index>
```

**Purpose:** Convert platform-neutral IR into VM-executable instructions.

### Phase 7: Virtual Machine Execution

**Input:** Bytecode instruction vector
**Output:** Program output (via PRINT instructions)

```
Bytecode → [execution loop] → stdout
  Stack: [2, 3, 5]
  Memory: {x: 10, y: 11}
  IP: (tracks current instruction index)
```

**Purpose:** Interpret bytecode and produce runtime behavior.

## 3. Data Flow

```
                  Source File
                      │
                      ↓
            ┌─────────────────────┐
            │  LEXER              │
            │  (lex/lexer.cpp)    │
            └─────────┬───────────┘
                      ↓
                [Token Stream]
                      ↓
            ┌─────────────────────┐
            │  PARSER             │
            │  (parser/parser.cp) │
            └─────────┬───────────┘
                      ↓
                  [AST Tree]
                      ↓
            ┌─────────────────────┐
            │  AST PRINTER        │  (diagnostic output)
            │  (ast/ast_printer)  │
            └─────────────────────┘
                      ↓
            ┌─────────────────────┐
            │  SEMANTIC ANALYZER  │
            │  (semantic/*.cpp)   │
            └─────────┬───────────┘
                      ↓
            [Validated AST + Symbol Table]
                      ↓
            ┌─────────────────────┐
            │  TAC GENERATOR      │
            │  (ir/tac_gen*.cpp)  │
            └─────────┬───────────┘
                      ↓
              [TAC Instructions]
                      ↓
            ┌─────────────────────┐
            │  BYTECODE GENERATOR │
            │ (bytecode/bc_gen) │
            └─────────┬───────────┘
                      ↓
                [Bytecode Vector]
                      ↓
            ┌─────────────────────┐
            │  VIRTUAL MACHINE    │
            │  (vm/vm.cpp)        │
            └─────────┬───────────┘
                      ↓
                  [stdout]
```

## 4. Module Structure

```
src/
├── lexer/
│   ├── token.h        (TokenType enum, Token struct)
│   ├── lexer.h        (Lexer class interface)
│   └── lexer.cpp      (Lexer implementation)
│
├── parser/
│   ├── parser.h       (Parser class interface)
│   └── parser.cpp     (Parser implementation)
│
├── ast/
│   ├── ast.h          (Statement & Expression class hierarchy)
│   └── ast_printer.h  (ASTPrinter diagnostic class)
│
├── semantic/
│   ├── semantic.h     (SemanticAnalyzer interface)
│   └── semantic.cpp   (SemanticAnalyzer implementation)
│
├── ir/
│   ├── tac.h          (TACOp enum, TACInstruction struct)
│   ├── tac_generator.h (TACGenerator interface)
│   └── tac_generator.cpp (TACGenerator implementation)
│
├── bytecode/
│   ├── bytecode.h     (OpCode enum, Instruction struct)
│   ├── bytecode_generator.h (BytecodeGenerator interface)
│   └── bytecode_generator.cpp (BytecodeGenerator impl)
│
├── vm/
│   ├── vm.h           (VM class interface)
│   └── vm.cpp         (VM execution loop)
│
├── utils/
│   └── logger.h       (diagnostic utilities)
│
└── main.cpp           (orchestrator: reads file, runs all phases)

tests/
├── lexer_tests.cpp    (per-phase unit test stubs)
└── vm_tests.cpp       (standalone VM bytecode tests)

docs/
├── architecture.md    (this file)
├── compiler_deep_dive.md (theory + implementation details)
└── language_spec.md   (grammar + semantics rules)
```

## 5. Dependency Graph

```
Lower-level (depended upon):
  token.h
     │
     ├← lexer.h/cpp
     │
     └← ast.h ← parser.h/cpp
                   │
                   └← semantic.h/cpp
                       │
                       └← tac.h ← tac_generator.h/cpp
                                    │
                                    └← bytecode.h ← bytecode_generator.h/cpp
                                                       │
                                                       └← vm.h/cpp

Higher-level (depends on others):
  main.cpp ← orchestrates all phases sequentially
```

**Key invariant:** Each component only depends on the outputs of previous
phases. This ensures:
- **Modularity**: Each phase can be tested independently
- **Clarity**: Data flow is unidirectional
- **Extensibility**: New backends (JIT, native codegen) can reuse earlier phases

## 6. Build & Execution

### CMake Project Structure

```cmake
cmake_minimum_required(VERSION 3.25)
project(LiveLy)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

# Main executable
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(lively ${SOURCES})

# Test executable (VM only)
add_executable(lively_vm_tests
    tests/vm_tests.cpp
    src/vm/vm.cpp
)

# CTest suite
include(CTest)
add_test(NAME vm_runtime COMMAND lively_vm_tests)
add_test(NAME lexer_hello COMMAND lively examples/hello.lv)
add_test(NAME parser_hello COMMAND lively examples/hello.lv)
# ... more tests ...
```

### Running the Compiler

```bash
# Compile & run all phases on a source file
./lively examples/hello.lv

# Output includes:
# [Phase 1] Lexer produced X tokens.
# [Phase 2] Parser produced Y statements.
# [Phase 3] AST pretty-printed
# [Phase 4] Semantic analysis passed
# [Phase 5] TAC generated - Z instructions.
# [Phase 6] Bytecode generated - W instructions.
# [Phase 8] Virtual machine executing bytecode...
# <program output>
# [OK] All compiler phases completed successfully.
```

### Running Tests

```bash
# Configure + Build + Test (one command)
cmake --workflow --preset ci

# Output:
# 1/10 Test  #1: vm_runtime .......................   Passed    0.01 sec
# 2/10 Test  #2: lexer_hello ......................   Passed    0.01 sec
# ...
# 10/10 Test #10: bytecode_loop ....................   Passed    0.01 sec
# 100% tests passed, 0 tests failed out of 10
```

## 7. Error Handling Strategy

Different phases catch different error categories:

| Error Type | Detected By | Example |
|------------|-------------|---------|
| **Syntax** | Parser | `bind x:int is` (missing expression) |
| **Type** | Semantic Analyzer | `bind x:int is alive;` (bool ≠ int) |
| **Scope** | Semantic Analyzer | `emit y;` (undeclared variable) |
| **Runtime** | Virtual Machine | Division by zero, stack underflow |

**Compilation stops** if any phase detects an error — no subsequent phases run.

## 8. Future Extensibility

The architecture supports future additions:

### Possible Optimizations
- **IR Optimization**: Insert optimizations between TAC and bytecode gen
- **Register Allocation**: Use the VM stack as a target for register assignment
- **Constant Folding**: Fold constants at TAC generation time

### Possible Backends
- **JIT Compilation**: Trace frequently-executed loops, compile to native code
- **Native Code Generation**: Emit assembly (x86, ARM, RISC-V)
- **IR Serialization**: Save bytecode to disk, reload for faster startup

### Possible Frontend Extensions
- **Lambda Expressions**: Add closures / higher-order functions
- **Module System**: Support #include or import statements
- **Macros**: Compile-time metaprogramming

All of these require only additions to specific phases — earlier phases remain unchanged.

## Summary

| Aspect | Design Choice | Rationale |
|--------|---------------|-----------|
| **Architecture** | Multi-phase compiler | Clear separation of concerns |
| **IR** | Three-Address Code | Linear, lowerable, optimizable |
| **Bytecode** | Stack-based | Simple, portable, compact |
| **Backend** | Tree-walking VM | Direct, understandable execution |
| **Language** | C++17 | Modern, type-safe, standard library |
| **Build System** | CMake | Cross-platform, widely supported |
| **Testing** | CTest + example files | Comprehensive regression suite |

The LiveLy compiler demonstrates a complete end-to-end implementation of a real compiler,
from lexical analysis through virtual machine execution, with all intermediate phases
documented and tested.
