# LiveLy Compiler — Deep Dive: Theory & Implementation

> A comprehensive explanation of how the Lexer, Parser, AST, Semantic Analyzer,
> Intermediate Representation, and Bytecode Generator work — covering both the
> Computer Science theory and the actual LiveLy implementation.

---

## Table of Contents

1. [Overview: The Compiler Pipeline](#1-overview-the-compiler-pipeline)
2. [Phase 1 — Lexical Analysis (Lexer)](#2-phase-1--lexical-analysis-lexer)
3. [Phase 2 — Parsing (Parser)](#3-phase-2--parsing-parser)
4. [Phase 3 — Abstract Syntax Tree (AST)](#4-phase-3--abstract-syntax-tree-ast)
5. [Phase 4 — Semantic Analysis](#5-phase-4--semantic-analysis)
6. [Phase 5 — Intermediate Representation (TAC)](#6-phase-5--intermediate-representation-tac)
7. [Phase 6 — Bytecode Generation](#7-phase-6--bytecode-generation)
8. [Phase 7 — Virtual Machine Execution](#8-phase-7--virtual-machine-execution)
9. [How They All Connect](#9-how-they-all-connect)

---

## 1. Overview: The Compiler Pipeline

### Theory

A compiler transforms human-readable source code into a form that can ultimately be
executed by a machine. This transformation happens in **stages** (also called **phases**),
where each stage takes the output of the previous stage and refines it further.

The classic compiler pipeline:

```
Source Code → Lexer → Parser → AST → Semantic → TAC (IR) → Bytecode → [VM...]
     |           |        |       |        |          |           |
  raw text    tokens    parse   tree   validated   3-address   stack-based
  (string)    (list)    tree  (memory)   tree       code       instructions
```

Each phase has a single, well-defined responsibility:

| Phase | Input | Output | Responsibility |
|-------|-------|--------|---------------|
| Lexer | Raw source string | Token stream | Break text into meaningful units |
| Parser | Token stream | AST (tree) | Enforce grammar rules / structure |
| AST | (data structure) | (data structure) | Represent program structure in memory |
| Semantic Analyzer | AST | Validated AST | Enforce meaning rules (types, scope) |
| TAC Generator | Validated AST | TAC instructions | Linearise tree into 3-address code |
| Bytecode Generator | TAC instructions | Bytecode | Produce stack-machine instructions |
| Virtual Machine | Bytecode | Program output | Execute bytecode instructions |

### LiveLy Implementation

In LiveLy, the pipeline is orchestrated by `main.cpp`:

```cpp
std::string source = readFile(argv[1]);       // raw text
Lexer lexer(source);
std::vector<Token> tokens = lexer.tokenize(); // → tokens
Parser parser(tokens);
auto ast = parser.parse();                    // → AST
ASTPrinter::printProgram(ast);                // → visual output
SemanticAnalyzer analyzer;
analyzer.analyze(ast);                        // → validated
TACGenerator tacGen;
auto tac = tacGen.generate(ast);              // → 3-address code
BytecodeGenerator bcGen;
auto bytecode = bcGen.generate(tac);          // → bytecode
VM vm(bytecode);
vm.run();                                      // → execution
```

Each component lives in its own directory and only depends on the component before it.

---

## 2. Phase 1 — Lexical Analysis (Lexer)

### Theory: What Is Lexical Analysis?

Lexical analysis (or **scanning**) is the process of converting a stream of characters
into a stream of **tokens**. A token is the smallest meaningful unit in a programming
language — like a word in a natural language sentence.

Consider this LiveLy line:
```
bind x:int is 10;
```

A human sees six distinct pieces: the keyword `bind`, a name `x`, a colon `:`,
a type `int`, the assignment word `is`, a number `10`, and a semicolon `;`.
The Lexer's job is to produce exactly this decomposition.

**Key concepts:**

- **Token**: A `(type, value, position)` tuple. Example: `(BIND, "bind", line 1, col 1)`.
- **Keyword**: A reserved word that has special meaning (`bind`, `emit`, `check`, etc.).
- **Identifier**: A user-defined name (`x`, `myVar`, `clamp`).
- **Literal**: A constant value embedded in source code (`10`, `alive`).
- **Lookahead**: Peeking at the next character without consuming it — needed for
  multi-character tokens like `>=` or `!=`.

**Finite State Machine (FSM) model:**

Theoretically, a lexer is a Deterministic Finite Automaton (DFA). At each step it reads
one character, transitions to a new state, and decides whether it has completed a token.
The states correspond to "reading a number", "reading an identifier", "reading an operator", etc.

```
START ──[letter]──→ IDENTIFIER state ──[non-alnum]──→ emit IDENTIFIER token
      ──[digit]───→ NUMBER state     ──[non-digit]──→ emit INT_LITERAL token
      ──[+]───────→ emit PLUS token
      ──[>]───────→ GREATER state    ──[=]──→ emit GREATER_EQUAL token
                                     ──[other]──→ emit GREATER token
```

### LiveLy Implementation

**Files:** `src/lexer/token.h`, `src/lexer/lexer.h`, `src/lexer/lexer.cpp`

#### Token Definition (`token.h`)

```cpp
enum class TokenType {
    // Keywords
    BIND, IS, EMIT, CHECK, OTHERWISE, CYCLE, FORGE, YIELD,
    // Types
    TYPE_INT, TYPE_BOOL,
    // Literals
    INT_LITERAL, BOOL_LITERAL,
    // Identifier
    IDENTIFIER,
    // Operators
    PLUS, MINUS, MULTIPLY, DIVIDE,
    GREATER, LESS, GREATER_EQUAL, LESS_EQUAL, EQUAL, NOT_EQUAL,
    // Assignment
    ASSIGN,  // "is"
    // Symbols
    COLON, SEMICOLON, COMMA, LPAREN, RPAREN, LBRACE, RBRACE,
    // Special
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};
```

LiveLy has 30 distinct token types. Each token stores its position (line + column)
for error reporting later in the pipeline.

#### The Scanner Engine (`lexer.cpp`)

The `Lexer` class maintains three pieces of state:
- `position` — current index into the source string
- `line` — current line number (for error messages)
- `column` — current column number

The `tokenize()` method implements the main DFA loop:

```cpp
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;  // handle trailing whitespace

        char c = currentChar();

        if (std::isalpha(c))       → readIdentifierOrKeyword()
        else if (std::isdigit(c))  → readNumber()
        else if (std::ispunct(c))  → readOperator() or readSymbol()
        else                       → throw error
    }

    tokens.push_back(EOF token);
    return tokens;
}
```

**Keyword detection** uses a hash map for O(1) lookup:

```cpp
static std::unordered_map<std::string, TokenType> keywords = {
    {"bind", TokenType::BIND},
    {"is",   TokenType::ASSIGN},
    {"emit", TokenType::EMIT},
    // ...
};
```

When `readIdentifierOrKeyword()` finishes reading an alphanumeric sequence like `bind`,
it checks the map. If found → keyword token. If not found → identifier token.

**Multi-character operators** use `peek()` (lookahead):

```cpp
case '>':
    if (peek() == '=') {
        advance(); advance();
        return Token(GREATER_EQUAL, ">=", ...);
    }
    advance();
    return Token(GREATER, ">", ...);
```

#### Example: Tokenizing `bind x:int is 10;`

```
Position 0: 'b' → isalpha → readIdentifierOrKeyword()
  reads "bind" → found in keywords → Token(BIND, "bind", 1, 1)

Position 5: 'x' → isalpha → readIdentifierOrKeyword()
  reads "x" → NOT in keywords → Token(IDENTIFIER, "x", 1, 6)

Position 6: ':' → ispunct → readSymbol()
  → Token(COLON, ":", 1, 7)

Position 7: 'i' → isalpha → readIdentifierOrKeyword()
  reads "int" → found in keywords → Token(TYPE_INT, "int", 1, 8)

Position 11: 'i' → isalpha → readIdentifierOrKeyword()
  reads "is" → found in keywords → Token(ASSIGN, "is", 1, 12)

Position 14: '1' → isdigit → readNumber()
  reads "10" → Token(INT_LITERAL, "10", 1, 15)

Position 16: ';' → ispunct → readSymbol()
  → Token(SEMICOLON, ";", 1, 17)

End of input → Token(END_OF_FILE, "", 1, 18)
```

**Output:** 8 tokens from 17 characters.

---

## 3. Phase 2 — Parsing (Parser)

### Theory: What Is Parsing?

Parsing (or **syntactic analysis**) takes the flat token stream and determines whether
it conforms to the language's **grammar** — the set of rules that define valid programs.
The output is a tree structure that represents the program's syntactic structure.

**Context-Free Grammar (CFG):**

Every programming language is defined by a grammar. For LiveLy:

```
program       → statement*
statement     → varDecl | assignment | emit | ifStmt | loop | function | return
varDecl       → BIND IDENTIFIER COLON type ASSIGN expression SEMICOLON
assignment    → IDENTIFIER ASSIGN expression SEMICOLON
emit          → EMIT expression SEMICOLON
ifStmt        → CHECK LPAREN expression RPAREN block (OTHERWISE block)?
loop          → CYCLE LPAREN expression RPAREN block
function      → FORGE IDENTIFIER LPAREN params RPAREN (COLON type)? block
return        → YIELD expression SEMICOLON

expression    → equality
equality      → comparison ( (EQUAL | NOT_EQUAL) comparison )*
comparison    → term ( (GREATER | LESS | GREATER_EQUAL | LESS_EQUAL) term )*
term          → factor ( (PLUS | MINUS) factor )*
factor        → primary ( (MULTIPLY | DIVIDE) primary )*
primary       → INT_LITERAL | BOOL_LITERAL | IDENTIFIER | LPAREN expression RPAREN
```

**Recursive Descent Parsing:**

LiveLy uses a **recursive descent parser** — one of the most common and intuitive parsing
techniques. Each grammar rule becomes a function:

```
parseStatement()  → dispatches to parseVarDecl(), parseEmit(), etc.
parseExpression() → calls parseEquality()
parseEquality()   → calls parseComparison(), handles ==, !=
parseComparison() → calls parseTerm(), handles >, <, >=, <=
parseTerm()       → calls parseFactor(), handles +, -
parseFactor()     → calls parsePrimary(), handles *, /
parsePrimary()    → handles literals, variables, grouped expressions
```

**Operator Precedence:**

The call hierarchy naturally enforces operator precedence. Functions called *deeper*
bind *tighter*:

```
Weakest   ==  !=           (parseEquality)
  ↓       >  <  >=  <=     (parseComparison)
  ↓       +  -             (parseTerm)
Tightest  *  /             (parseFactor)
```

So `2 + 3 * 4` is parsed as `2 + (3 * 4)` because `parseTerm` calls `parseFactor`
first, which grabs `3 * 4` before `parseTerm` sees the `+`.

### LiveLy Implementation

**Files:** `src/parser/parser.h`, `src/parser/parser.cpp`

#### Core Navigation Methods

```cpp
Token peek() const;      // Look at current token without consuming
Token advance();         // Consume current token, return it
bool check(TokenType);   // Does current token match this type?
bool match(TokenType);   // If current matches, consume and return true
Token consume(TokenType, string errorMessage);  // Expect this type or throw
```

#### Statement Dispatch (`parseStatement`)

```cpp
std::unique_ptr<Statement> Parser::parseStatement() {
    if (match(TokenType::BIND))       return parseVarDecl();
    if (match(TokenType::EMIT))       return parseEmit();
    if (match(TokenType::CHECK))      return parseIf();
    if (match(TokenType::CYCLE))      return parseLoop();
    if (match(TokenType::FORGE))      return parseFunction();
    if (match(TokenType::YIELD))      return parseReturn();
    if (check(TokenType::IDENTIFIER)) return parseAssignment();
    throw runtime_error("Unexpected statement ...");
}
```

The first token of each statement uniquely identifies its type.  This is called
the **FIRST set** in grammar theory — it's what makes LiveLy's grammar LL(1)
(parseable with just one token of lookahead).

#### Expression Parsing Example

For the expression `2 + 3 * (4 - 1)`:

```
parseExpression()
  └→ parseEquality()
       └→ parseComparison()
            └→ parseTerm()
                 ├→ parseFactor()
                 │    └→ parsePrimary() → Literal(2)
                 │  [sees +, loops]
                 └→ parseFactor()
                      ├→ parsePrimary() → Literal(3)
                      │  [sees *, loops]
                      └→ parsePrimary()
                           └→ [sees '('] → parseExpression() recursively
                                 └→ parseTerm()
                                      ├→ Literal(4)
                                      │  [sees -]
                                      └→ Literal(1)
                                      → BinaryExpr("-", 4, 1)
                           → BinaryExpr("*", 3, (4-1))
                 → BinaryExpr("+", 2, (3*(4-1)))
```

#### Function Declaration Parsing

For `forge clamp(n:int, limit:int): int { ... }`:

```cpp
Token name = consume(IDENTIFIER);  // "clamp"
consume(LPAREN);
// Parse parameter list
do {
    Token paramName = consume(IDENTIFIER);  // "n", "limit"
    consume(COLON);
    Token paramType = advance();            // "int", "int"
    parameters.push_back({paramName, paramType});
} while (match(COMMA));
consume(RPAREN);

// Optional return type
if (match(COLON)) {
    returnType = advance();  // "int"
}

consume(LBRACE);
body = parseBlock();  // recursively parse statements until '}'
```

---

## 4. Phase 3 — Abstract Syntax Tree (AST)

### Theory: What Is an AST?
 
The Abstract Syntax Tree is the central data structure of a compiler. It represents
the **logical structure** of a program, stripped of syntactic noise (semicolons,
parentheses, keywords). Every node in the tree is either a **statement** (does something)
or an **expression** (produces a value).

**Parse Tree vs AST:**

A parse tree mirrors the grammar exactly — every production rule becomes a node.
An AST simplifies this by removing unnecessary intermediate nodes:

```
Parse tree for "2 + 3":        AST for "2 + 3":
     term                        BinaryExpr(+)
    / | \                         /         \
factor + factor              Literal(2)  Literal(3)
  |       |
  2       3
```

The AST only keeps what matters for execution.

**Node Hierarchy (typical):**

```
ASTNode (abstract base)
  ├── Expression (produces a value)
  │    ├── LiteralExpr   — e.g., 42, "alive"
  │    ├── VariableExpr  — e.g., x, myVar
  │    └── BinaryExpr    — e.g., a + b, x >= y
  └── Statement (performs an action)
       ├── VarDecl       — bind x:int is 10;
       ├── Assignment    — x is 20;
       ├── EmitStmt      — emit x;
       ├── IfStmt        — check (...) { } otherwise { }
       ├── LoopStmt      — cycle (...) { }
       ├── ReturnStmt    — yield value;
       └── FunctionDecl  — forge name(...) { }
```

### LiveLy Implementation

**Files:** `src/ast/ast.h`, `src/ast/ast_printer.h`

#### Node Classes (`ast.h`)

Every node uses `std::unique_ptr` for ownership — when a parent is destroyed, its
children are automatically deallocated. This prevents memory leaks without a garbage
collector.

```cpp
class BinaryExpr : public Expression {
public:
    std::string op;                    // "+", "-", ">=", etc.
    std::unique_ptr<Expression> left;  // left operand (owned)
    std::unique_ptr<Expression> right; // right operand (owned)
};

class IfStmt : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    std::vector<std::unique_ptr<Statement>> elseBranch;
};

class FunctionDecl : public Statement {
public:
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters;  // (name, type)
    std::string returnType;
    std::vector<std::unique_ptr<Statement>> body;
};
```

#### AST Printer (`ast_printer.h`)

To verify the tree is correct, the AST Printer walks every node using `dynamic_cast`
to identify its concrete type, then prints it with indentation proportional to depth.

For `hello.lv`, the printer produces:

```
[0] VarDecl: x : int
      +-- Literal: 10
[1] VarDecl: y : int
      +-- BinaryOp: +
            +-- Left:  Literal: 2
            +-- Right: BinaryOp: *
                  +-- Left:  Literal: 3
                  +-- Right: BinaryOp: -
                        +-- Left:  Literal: 4
                        +-- Right: Literal: 1
[2] VarDecl: aliveflag : bool
      +-- Literal: alive
[3] Assignment: x
      +-- BinaryOp: /
            +-- Left:  Variable: x
            +-- Right: Variable: y
[4] IfStmt (check):
      +-- Condition: BinaryOp: >=  (Variable: x, Literal: 5)
      +-- Then:      Emit → Variable: x
      +-- Otherwise: Emit → Literal: 0
[5] LoopStmt (cycle):
      +-- Condition: BinaryOp: !=  (Variable: x, Literal: 0)
      +-- Body:      Assignment: x → BinaryOp: - (Variable: x, Literal: 1)
[6] FunctionDecl: clamp -> int
      +-- Params:  n:int, limit:int
      +-- Body:    IfStmt → yield n / yield limit
```

This visualisation proves:
- Operator precedence is correct (`*` binds tighter than `+`)
- Parenthesized grouping works (`(4 - 1)` is a subtree)
- Control flow nesting is preserved (yield inside if, inside function)

---

## 5. Phase 4 — Semantic Analysis

### Theory: What Is Semantic Analysis?

The Parser ensures the program is **syntactically** valid — it follows the grammar rules.
But syntax alone doesn't guarantee **meaning**. Semantic analysis checks that the program
makes logical sense.

**Three categories of semantic errors:**

| Category | Example | Why Syntax Can't Catch It |
|----------|---------|--------------------------|
| **Type Errors** | `bind x:int is alive;` | Grammar allows any expr after `is` |
| **Scope Errors** | `emit y;` (y never declared) | Grammar allows any identifier |
| **Declaration Errors** | Two `bind x:int` in same scope | Grammar has no memory |

**The Symbol Table:**

The central data structure for semantic analysis is the **symbol table** — a mapping
from variable names to their types and scope information. When processing a declaration,
we add to the table. When processing a usage, we look up from the table.

**Scoping:**

Most languages support nested scopes — a variable declared inside a function is not
visible outside. This is modeled with a **scope stack**:

```
Global Scope:  { x → int, aliveflag → bool, clamp → func }
  │
  └─ Function "clamp" Scope:  { n → int, limit → int }
```

When looking up a variable, we search from the innermost scope outward. This implements
**lexical scoping** — the foundational scoping rule used by C, Java, Python, and LiveLy.

**Type Inference for Expressions:**

To check `bind x:int is 2 + 3;`, the analyzer must determine that `2 + 3` evaluates
to type `int`. This requires recursive type inference:

```
typeOf(2 + 3)
  = typeOf(BinaryExpr("+", Literal(2), Literal(3)))
  = check that typeOf(Literal(2)) == "int"  ✓
    check that typeOf(Literal(3)) == "int"  ✓
    "+" requires int operands → returns "int"
```

**Comparison operators** are special — they take `int` operands but return `bool`:

```
typeOf(x >= 5)
  = typeOf(BinaryExpr(">=", Variable(x), Literal(5)))
  = lookup(x) → "int"
    typeOf(Literal(5)) → "int"
    ">=" requires int operands → returns "bool"
```

### LiveLy Implementation

**Files:** `src/semantic/semantic.h`, `src/semantic/semantic.cpp`

#### Scope Stack

```cpp
class SemanticAnalyzer {
    std::vector<std::unordered_map<std::string, std::string>> scopes;
    std::string currentReturnType;  // tracks expected return type in functions
};
```

Instead of a flat symbol table, LiveLy uses a **stack of scopes**. This correctly
handles function parameters being visible only inside the function body.

```cpp
void SemanticAnalyzer::declare(const std::string& name, const std::string& type) {
    auto& current = scopes.back();
    if (current.find(name) != current.end()) {
        error("Variable already declared in this scope: " + name);
    }
    current[name] = type;
}

std::string SemanticAnalyzer::lookup(const std::string& name) {
    for (int i = scopes.size() - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return it->second;
    }
    error("Undefined variable: " + name);
}
```

`lookup()` walks **inside-out** — checking the current scope first, then progressively
outer scopes. This means a local variable with the same name as a global one will
**shadow** the global — exactly like in C/C++.

#### Statement Analysis

Each statement type triggers specific checks:

**Variable Declaration (`bind x:int is 10;`):**
```cpp
if (auto var = dynamic_cast<VarDecl*>(stmt)) {
    std::string valueType = analyzeExpression(var->value.get());
    if (valueType != var->type) {
        error("Type mismatch: expected " + var->type + ", got " + valueType);
    }
    declare(var->name, var->type);
}
```
1. Infer the type of the right-hand expression.
2. Compare it against the declared type.
3. If they match, register the variable in the current scope.

**Function Declaration (`forge clamp(n:int, limit:int): int { ... }`):**
```cpp
if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {
    declare(fn->name, "func");           // register function in current scope
    pushScope();                          // new scope for function body
    for (auto& param : fn->parameters)
        declare(param.first, param.second); // register params
    currentReturnType = fn->returnType;   // track for yield-checking
    for (auto& s : fn->body)
        analyzeStatement(s.get());        // analyze each statement in body
    popScope();                           // exit function scope
}
```

This ensures function parameters are visible inside the function but not outside,
and that `yield` statements return the correct type.

#### Expression Type Inference

```cpp
std::string SemanticAnalyzer::analyzeExpression(Expression* expr) {
    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        if (lit->value == "alive" || lit->value == "dead") return "bool";
        return "int";
    }
    else if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        return lookup(var->name);  // consult the scope stack
    }
    else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        std::string left  = analyzeExpression(bin->left.get());
        std::string right = analyzeExpression(bin->right.get());

        if (op is arithmetic)  → require int + int → return int
        if (op is comparison)  → require int + int → return bool
        if (op is equality)    → require matching  → return bool
    }
}
```

#### What Gets Caught

| LiveLy Code | Error Detected |
|-------------|---------------|
| `bind x:int is alive;` | Type mismatch: expected int, got bool |
| `emit y;` | Undefined variable: y |
| `bind x:int is 10; bind x:int is 20;` | Variable already declared: x |
| `check (10) { ... }` | Condition must be bool, got int |
| `x is alive;` (where x is int) | Type mismatch in assignment: expected int, got bool |
| `yield 10;` (in a bool function) | Return type mismatch: expected bool, got int |

---

## 6. Phase 5 — Intermediate Representation (TAC)

### Theory: What Is an Intermediate Representation?

After the frontend validates that a program is syntactically and semantically correct,
the compiler must **lower** the tree into a flat, linear form that is closer to machine
execution. This intermediate representation (IR) sits between the high-level AST and
the low-level bytecode.

The most common IR is **Three-Address Code (TAC)**, where every instruction has at most
three operands: a result, and up to two arguments.

```
t0 = 4 - 1       // SUB t0, 4, 1
t1 = 3 * t0       // MUL t1, 3, t0
t2 = 2 + t1       // ADD t2, 2, t1
y  = t2           // ASSIGN y, t2
```

**Key properties of TAC:**

- Each instruction performs exactly one operation.
- Complex expressions are broken into atomic steps using **temporary variables** (`t0`, `t1`, ...).
- Control flow uses **labels** and **jumps** instead of nested blocks.
- The flat list is easy to optimise, reorder, and translate to machine code.

**Control flow lowering:**

Nested `if/else` and `loop` blocks are converted to labels and conditional jumps:

```
// Source: check (x >= 5) { emit x; } otherwise { emit 0; }
t4 = x >= 5
IF_FALSE L0, t4       // if t4 is false, jump to L0
EMIT x
GOTO L1               // skip else branch
LABEL L0              // else branch starts
EMIT 0
LABEL L1              // end of if
```

### LiveLy Implementation

**Files:** `src/ir/tac.h`, `src/ir/tac_generator.h`, `src/ir/tac_generator.cpp`

#### TAC Instruction Format (`tac.h`)

```cpp
enum class TACOp {
    ADD, SUB, MUL, DIV,           // arithmetic
    GT, LT, GE, LE, EQ, NEQ,     // comparison
    ASSIGN,                        // variable assignment
    LABEL, GOTO, IF_FALSE,        // control flow
    EMIT                           // output
};

struct TACInstruction {
    TACOp op;
    std::string result;   // destination or label name
    std::string arg1;     // first operand
    std::string arg2;     // second operand (binary ops)
};
```

#### Generator Architecture (`tac_generator.cpp`)

The generator walks the AST recursively, maintaining two counters:
- `tempCounter` — produces fresh temporaries (`t0`, `t1`, `t2`, ...)
- `labelCounter` — produces fresh labels (`L0`, `L1`, `L2`, ...)

**Expression lowering** is the key operation — every complex expression is
decomposed into a sequence of TAC instructions that store intermediate results
in temporaries:

```cpp
std::string TACGenerator::generateExpression(Expression* expr) {
    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        std::string left  = generateExpression(bin->left.get());
        std::string right = generateExpression(bin->right.get());
        std::string temp  = newTemp();  // t0, t1, ...
        instructions.emplace_back(op, temp, left, right);
        return temp;  // caller uses this temporary
    }
    // ...
}
```

**Loop lowering** generates the classic label-jump pattern:

```cpp
// cycle (condition) { body }
std::string startLabel = newLabel();  // L2
std::string endLabel   = newLabel();  // L3

instructions.emplace_back(TACOp::LABEL, startLabel);
std::string cond = generateExpression(loop->condition.get());
instructions.emplace_back(TACOp::IF_FALSE, endLabel, cond);
// ... generate body ...
instructions.emplace_back(TACOp::GOTO, startLabel);
instructions.emplace_back(TACOp::LABEL, endLabel);
```

> **Note:** `FunctionDecl` and `ReturnStmt` are not yet lowered to IR — the
> generator skips them with a warning. This will be addressed when `CALL`/`RET`
> opcodes are added.

#### Example: `hello.lv` TAC Output

```
ASSIGN x 10
SUB t0 4 1
MUL t1 3 t0
ADD t2 2 t1
ASSIGN y t2
ASSIGN aliveflag alive
DIV t3 x y
ASSIGN x t3
GE t4 x 5
IF_FALSE L0 t4
EMIT x
GOTO L1
LABEL L0
EMIT 0
LABEL L1
LABEL L2
NEQ t5 x 0
IF_FALSE L3 t5
SUB t6 x 1
ASSIGN x t6
GOTO L2
LABEL L3
```

---

## 7. Phase 6 — Bytecode Generation

### Theory: What Is Bytecode?

Bytecode is a compact, machine-oriented instruction set designed for a **stack-based
virtual machine**. Unlike TAC (which uses named temporaries), bytecode operates on
an implicit **operand stack** — values are pushed onto the stack and operators pop
their arguments from it.

```
TAC:        ADD t2, 2, t1        (uses named operands)
Bytecode:   PUSH_CONST 2         (push 2 onto stack)
            LOAD t1              (push value of t1 onto stack)
            ADD                  (pop two, push result)
            STORE t2             (pop result into t2)
```

**Key design decisions:**

- **`PUSH_CONST`** vs **`LOAD`**: Numeric literals and boolean constants (`alive`/`dead`)
  use `PUSH_CONST`. Named variables use `LOAD`. This distinction is critical — without
  it, the VM cannot tell whether `10` means "the number ten" or "a variable called 10".
- **Jump targets are bytecode indices**: Labels from TAC are resolved to concrete
  instruction addresses during a two-pass compilation.

### LiveLy Implementation

**Files:** `src/bytecode/bytecode.h`, `src/bytecode/bytecode_generator.h`, `src/bytecode/bytecode_generator.cpp`

#### Instruction Set (`bytecode.h`)

```cpp
enum class OpCode {
    PUSH_CONST,          // push a literal value
    LOAD,                // push a variable's value
    STORE,               // pop and store into variable

    ADD, SUB, MUL, DIV,  // arithmetic (pop 2, push 1)
    GT, LT, GE, LE,     // comparison (pop 2, push bool)
    EQ, NEQ,

    JUMP,                // unconditional jump
    JUMP_IF_FALSE,       // conditional jump (pop 1)

    PRINT                // pop and output
};
```

#### Two-Pass Compilation

The bytecode generator uses a **two-pass** approach:

**Pass 1 — Label Resolution:** Scans all TAC instructions and computes the bytecode
index each label maps to. This is non-trivial because a single TAC instruction may
expand to multiple bytecode instructions:

| TAC Opcode | Bytecode Instructions | Count |
|------------|----------------------|-------|
| `ASSIGN` | `PUSH_CONST`/`LOAD` + `STORE` | 2 |
| `ADD` (etc.) | `LOAD` + `LOAD` + `OP` + `STORE` | 4 |
| `IF_FALSE` | `LOAD` + `JUMP_IF_FALSE` | 2 |
| `GOTO` | `JUMP` | 1 |
| `EMIT` | `LOAD` + `PRINT` | 2 |
| `LABEL` | *(none — marker only)* | 0 |

```cpp
void BytecodeGenerator::firstPass(const std::vector<TACInstruction>& tac) {
    int index = 0;
    for (const auto& instr : tac) {
        if (instr.op == TACOp::LABEL) {
            labelMap[instr.result] = index;
        } else {
            index += bytecodeCountForTAC(instr.op);
        }
    }
}
```

**Pass 2 — Code Emission:** Walks through TAC again, emitting bytecode
instructions. The `emitLoad()` helper distinguishes constants from variables:

```cpp
void BytecodeGenerator::emitLoad(const std::string& operand) {
    if (isNumber(operand) || operand == "alive" || operand == "dead")
        instructions.emplace_back(OpCode::PUSH_CONST, operand);
    else
        instructions.emplace_back(OpCode::LOAD, operand);
}
```

#### Example: `hello.lv` Bytecode Output (excerpt)

```
 0: PUSH_CONST 10        // bind x:int is 10
 1: STORE x
 2: PUSH_CONST 4         // sub-expression: 4 - 1
 3: PUSH_CONST 1
 4: SUB
 5: STORE t0
 ...
28: LOAD t4              // check (x >= 5)
29: JUMP_IF_FALSE 33     // → else branch at bytecode index 33
30: LOAD x               // emit x (then branch)
31: PRINT
32: JUMP 35              // skip else
33: PUSH_CONST 0         // emit 0 (else branch)
34: PRINT
35: LOAD x               // cycle (x != 0)
 ...
40: JUMP_IF_FALSE 48     // → loop exit
 ...
47: JUMP 35              // → back to loop start
```

Jump targets (33, 35, 48) are resolved correctly by `firstPass` — each points
to the exact bytecode index of the target instruction.

---

## 8. Phase 7 — Virtual Machine Execution

### Theory: What Is a Virtual Machine?

A **virtual machine (VM)** is a software abstraction that simulates a hardware processor.
Instead of executing machine code directly on CPU-specific instruction sets (like x86 or ARM),
a VM executes a **portable bytecode** — instructions that are independent of the underlying
hardware.

**Why use a VM?**

1. **Portability**: Compile once to bytecode, run on any machine with a VM implementation.
2. **Safety**: The VM can enforce memory bounds, type checking, and sandbox untrusted code.
3. **Simplicity**: VM design is simpler than generating native machine code.
4. **Flexibility**: Can easily add new features (GC, JIT, profiling) in the VM layer.

**Stack-Based VMs:**

LiveLy uses a **stack-based architecture** — the most common model:

```
Stack (implicit)           Memory (variables)
┌───────────────────────┐   ┌──────────────────┐
│ (top)      [sp]       │   │ x    = 10        │
│            [sp-1]     │   │ y    = 11        │
│            [sp-2]     │   │ temp = 5         │
│ (bottom)   [sp-...]   │   │ ...              │
└───────────────────────┘   └──────────────────┘
```

**Instruction categories:**

| Category | Examples | Effect |
|----------|----------|--------|
| **Stack Ops** | `PUSH_CONST`, `LOAD`, `STORE` | Manage stack & memory |
| **Arithmetic** | `ADD`, `SUB`, `MUL`, `DIV` | Pop 2, compute, push 1 |
| **Comparison** | `GT`, `LT`, `GE`, `LE`, `EQ`, `NEQ` | Pop 2, compute bool, push 1 |
| **Control Flow** | `JUMP`, `JUMP_IF_FALSE` | Change instruction pointer |
| **Output** | `PRINT` | Pop and print to stdout |

**Execution Loop (pseudocode):**

```
ip = 0
while ip < bytecode.length:
    instr = bytecode[ip]
    switch instr.opcode:
        case PUSH_CONST:  stack.push(instr.operand)
        case LOAD:        stack.push(memory[instr.operand])
        case ADD:         stack.push(stack.pop() + stack.pop())
        case JUMP:        ip = instr.target; continue
        case JUMP_IF_FALSE:
            cond = stack.pop()
            if (!cond) ip = instr.target; continue
        case PRINT:       cout << stack.pop()
    ip++
```

The VM repeatedly fetches an instruction, executes it, and advances the instruction pointer
until the bytecode is exhausted.

### LiveLy Implementation

**Files:** `src/vm/vm.h`, `src/vm/vm.cpp`

#### VM State

```cpp
class VM {
private:
    std::vector<Instruction> instructions;  // bytecode
    std::vector<int> stack;                  // operand stack
    std::unordered_map<std::string, int> memory;  // variable storage
    int ip = 0;                              // instruction pointer
};
```

Three key members:
- **`instructions`**: The bytecode program — immutable after construction.
- **`stack`**: The operand stack — grows/shrinks as instructions execute.
- **`memory`**: Variable bindings — stores the runtime value of every declared variable.
- **`ip`**: Instruction pointer — tracks the current bytecode index (advanced after each instruction).

#### Stack Operations

```cpp
void VM::push(int value) {
    stack.push_back(value);
}

int VM::pop() {
    if (stack.empty()) throw std::runtime_error("Stack underflow");
    int value = stack.back();
    stack.pop_back();
    return value;
}
```

These are minimal stubs — safety checking (underflow detection) is in place.

#### Operand Resolution

```cpp
int VM::resolveValue(const std::string& operand) {
    if (operand.empty()) return 0;

    // Check if operand is a numeric literal
    if (std::isdigit(operand[0]) || 
        (operand[0] == '-' && operand.size() > 1 && std::isdigit(operand[1]))) {
        return std::stoi(operand);
    }

    // Otherwise, look up variable in memory
    if (memory.find(operand) == memory.end()) {
        throw std::runtime_error("Undefined variable: " + operand);
    }
    return memory[operand];
}
```

This helper handles both numeric literals (via `stoi()`) and variable names (via memory lookup).

#### Execution Loop

```cpp
void VM::run() {
    while (ip < instructions.size()) {
        const Instruction& instr = instructions[ip];

        switch (instr.op) {
        // ---- Stack / Memory ----
        case OpCode::PUSH_CONST:
            if (instr.operand == "alive")      push(1);
            else if (instr.operand == "dead")  push(0);
            else                               push(std::stoi(instr.operand));
            break;

        case OpCode::LOAD:
            push(resolveValue(instr.operand));
            break;

        case OpCode::STORE: {
            int value = pop();
            memory[instr.operand] = value;
            break;
        }

        // ---- Arithmetic ----
        case OpCode::ADD: {
            int b = pop();
            int a = pop();
            push(a + b);
            break;
        }
        // (SUB, MUL, DIV follow the same pattern)

        // ---- Comparisons ----
        case OpCode::GT: {
            int b = pop();
            int a = pop();
            push(a > b);  // C++ bool implicitly converts to 1/0
            break;
        }
        // (LT, GE, LE, EQ, NEQ follow the same pattern)

        // ---- Control Flow ----
        case OpCode::JUMP:
            ip = std::stoi(instr.operand);
            continue;  // skip the ip++ at the end

        case OpCode::JUMP_IF_FALSE: {
            int condition = pop();
            if (!condition) {
                ip = std::stoi(instr.operand);
                continue;  // skip the ip++
            }
            break;
        }

        // ---- Output ----
        case OpCode::PRINT: {
            int value = pop();
            std::cout << value << std::endl;
            break;
        }

        default:
            throw std::runtime_error("Unknown opcode");
        }

        ip++;  // advance to next instruction (unless control flow changed it)
    }
}
```

**Key details:**

- **Boolean literals:** `alive` → `1`, `dead` → `0` (native C++ `int`).
- **Stack discipline:** Arithmetic ops pop their arguments **in reverse order**.
  For `ADD`, we pop `b` first, then `a`, so the result is `a + b` (not `b + a`).
- **Control flow:** `JUMP` and `JUMP_IF_FALSE` use `continue` to skip the `ip++` at the
  loop's end — the instruction pointer is already set to the jump target.
- **In-order error checking:** Variable lookups fail at runtime if used before declaration
  (runtime error, not a compile-time semantic error).

#### Example Execution: `emit 0;`

Bytecode:
```
0: PUSH_CONST 0
1: PRINT
```

Execution trace:
```
ip=0:  PUSH_CONST 0    stack=[0],   memory={}
ip=1:  PRINT           stack=[],    memory={},   output: "0\n"
ip=2:  (end)
```

#### Example Execution: Loop from `loop_test.lv`

Bytecode:
```
 0: PUSH_CONST 0
 1: STORE i
 2: LOAD i
 3: PUSH_CONST 10
 4: LT
 5: STORE t0
 6: LOAD t0
 7: JUMP_IF_FALSE 15
 8: LOAD i
 9: PUSH_CONST 1
10: ADD
11: STORE t1
12: LOAD t1
13: STORE i
14: JUMP 2
15: LOAD i
16: PRINT
```

Execution summary (simplified):
```
ip=0-1:  i = 0                   memory={i:0}
ip=2-5:  t0 = i < 10             memory={i:0, t0:1}
ip=6-7:  jump_if_false 15        (cond=1, continue)
ip=8-13: i = i + 1               memory={i:1, t0:?}
ip=14:   jump 2                  (loop back)

[Loop repeats until i=10]

ip=2-5:  t0 = 10 < 10            memory={i:10, t0:0}
ip=6-7:  jump_if_false 15        (cond=0, jump to 15)
ip=15-16: emit i                 output: "10\n"
```

### Integration with the Compiler

In `main.cpp`, the VM is invoked immediately after bytecode generation:

```cpp
BytecodeGenerator bcGen;
auto bytecode = bcGen.generate(tac);          // → bytecode

std::cout << "\n[Phase 8] Virtual machine executing bytecode...\n";
VM vm(bytecode);
vm.run();                                      // → execution
```

No separate compilation step is needed — the VM receives the bytecode vector directly
and begins executing it.

### Testing the VM

A standalone test executable (`lively_vm_tests`) verifies VM behavior independently
of the compiler pipeline:

```cpp
// Minimal test: 2 + 3 * 4 = 20
const std::vector<Instruction> program = {
    Instruction(OpCode::PUSH_CONST, "2"),
    Instruction(OpCode::PUSH_CONST, "3"),
    Instruction(OpCode::PUSH_CONST, "4"),
    Instruction(OpCode::MUL),       // 3 * 4 = 12
    Instruction(OpCode::ADD),       // 2 + 12 = 14
    Instruction(OpCode::PRINT)
};

VM vm(program);
vm.run();  // outputs: 14
```

This test runs entirely in isolation from lexer/parser/compiler phases, ensuring
the VM's core execution logic is sound.

---

## 9. How They All Connect

### The Complete Data Flow

```
                    "bind x:int is 10;"
                           │
                    ┌──────┴──────┐
                    │    LEXER    │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
        Token(BIND)  Token(IDENT,"x") Token(INT_LITERAL,"10") ...
              │            │            │
              └────────────┼────────────┘
                           │
                    ┌──────┴──────┐
                    │   PARSER    │
                    └──────┬──────┘
                           │
                     VarDecl Node
                    ┌──────┴──────┐
                    │ name: "x"   │
                    │ type: "int" │
                    │ value: ──────┼──→ LiteralExpr("10")
                    └─────────────┘
                           │
                    ┌──────┴──────┐
                    │  SEMANTIC   │
                    └──────┬──────┘
                           │
              typeOf(LiteralExpr("10")) → "int"
              declared type: "int"  →  ✓ PASS
                           │
                    ┌──────┴──────┐
                    │  TAC (IR)   │
                    └──────┬──────┘
                           │
              ASSIGN x 10   (flat linear instruction)
                           │
                    ┌──────┴──────┐
                    │  BYTECODE   │
                    └──────┬──────┘
                           │
              0: PUSH_CONST 10
              1: STORE x
```

### Module Dependency Graph

```
token.h ◄──── lexer.h/cpp
   │
   └──────── ast.h ◄──── ast_printer.h
               │
               ├──── parser.h/cpp
               │
               ├──── semantic.h/cpp
               │
               ├──── tac.h ◄──── tac_generator.h/cpp
               │                       │
               │              bytecode.h ◄──── bytecode_generator.h/cpp
               │
               └──── main.cpp (orchestrator)
```

Every module only depends on the modules above it in this graph. The IR layer
depends on `ast.h` (for AST node types) and the Bytecode layer depends on
`tac.h` (for TAC instruction types). This strict layering means any module
can be tested independently.

### CMake Test Integration

```cmake
Test 1: lexer_hello            → validates token stream output
Test 2: parser_hello           → validates AST generation + pipeline completion
Test 3: semantic_hello         → validates type-safe code passes analysis
Test 4: semantic_type_mismatch → validates "bind x:int is alive;" is rejected
Test 5: semantic_undefined     → validates "emit y;" (undeclared) is rejected
Test 6: tac_hello              → validates TAC contains ASSIGN + EMIT
Test 7: tac_loop               → validates TAC contains LABEL + IF_FALSE + GOTO
Test 8: bytecode_hello         → validates bytecode contains PUSH_CONST + STORE + PRINT
Test 9: bytecode_loop          → validates bytecode contains JUMP_IF_FALSE + JUMP
```

Running `cmake --workflow --preset ci` executes all 9 tests in one command,
giving you a complete regression suite that verifies every phase of the compiler
pipeline from end to end.

---

## Summary

| Phase | Theory | LiveLy Implementation | Key File |
|-------|--------|----------------------|-----------|
| Lexer | DFA / Finite State Machine | Character-by-character scanner with keyword hash map | `lexer.cpp` |
| Parser | LL(1) Recursive Descent | One function per grammar rule, precedence via call depth | `parser.cpp` |
| AST | Tree data structure | C++17 class hierarchy with `unique_ptr` ownership | `ast.h` |
| Semantic | Symbol Table + Type System | Scope stack with inside-out lookup and recursive type inference | `semantic.cpp` |
| TAC (IR) | Three-Address Code | Linearises AST into flat instructions with temps and labels | `tac_generator.cpp` |
| Bytecode | Stack-machine instructions | Two-pass compiler: label resolution then code emission | `bytecode_generator.cpp` |
| Virtual Machine | Stack-based interpreter | Execute bytecode with operand stack + variable memory | `vm.cpp` |

Each phase transforms the program into a progressively more refined representation,
catching different classes of errors along the way. Phases 1–4 form the **frontend**
(validation and structure), Phases 5–6 form the **middle-end** (lowering to
executable form), and Phase 7 forms the **backend** (execution via VM). The complete
pipeline runs automatically when `lively <source_file>` is executed or tests are run.