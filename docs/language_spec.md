# LiveLy Language Specification

Version: 0.4 (current implementation)

## 1. Overview

LiveLy is a statically typed procedural language with explicit declarations, expression-based assignments, block-structured control flow, and function declarations.

Current compiler pipeline:

1. Lexical analysis (tokenization)
2. Parsing (AST construction)
3. Semantic analysis (type and symbol checks)
4. Intermediate representation (Three-Address Code generation)
5. Bytecode generation (stack-machine instructions)
6. Virtual machine execution (bytecode interpretation)

The implementation validates programs through phases 1–3, lowers to bytecode in phases 4–5,
and executes via a stack-based virtual machine in phase 6. Compiler diagnostics are printed at each stage.

## 2. Design Principles

1. Explicit typing: every variable declaration must include a type.
2. Strong static checks: type mismatches are rejected before execution.
3. Predictable syntax: statement-oriented grammar with explicit delimiters (`;`, `{}`, `()`).
4. Small core: only essential constructs are included in v0.4.

## 3. Lexical Specification

### 3.1 Character Model

Source text is read as a character stream.

Whitespace is ignored except as a token separator.

### 3.2 Reserved Keywords

Language keywords:

- `bind` (variable declaration)
- `is` (assignment operator word)
- `emit` (output statement)
- `check` (if condition)
- `otherwise` (else branch)
- `cycle` (loop statement)
- `forge` (function declaration)
- `yield` (return statement)

Type keywords:

- `int`
- `bool`

Boolean literals:

- `alive` (true)
- `dead` (false)

### 3.3 Identifiers

Identifier rules in the current lexer:

1. First character must be alphabetic (`A-Z` or `a-z`).
2. Remaining characters may be alphanumeric (`A-Z`, `a-z`, `0-9`).
3. `_` is not accepted.
4. Reserved keywords cannot be used as identifiers.

Examples:

- Valid: `x`, `aliveflag`, `clamp1`
- Invalid: `_x`, `2nd`, `my_var`

### 3.4 Literals

1. Integer literals: one or more digits, base-10 (example: `0`, `42`, `1000`).
2. Boolean literals: `alive` and `dead`.

### 3.5 Operators and Symbols

Arithmetic operators:

- `+`, `-`, `*`, `/`

Comparison operators:

- `>`, `<`, `>=`, `<=`

Equality operators:

- `==`, `!=`

Assignment keyword:

- `is`

Punctuation:

- `:` `;` `,` `(` `)` `{` `}`

### 3.6 End of File Token

The lexer appends an explicit end-of-file token after tokenization completes.

## 4. Grammar (EBNF)

The following grammar describes v0.4 behavior:

```ebnf
program          = { statement } ;

statement        = var_decl
				 | assignment
				 | emit_stmt
				 | if_stmt
				 | loop_stmt
				 | function_decl
				 | return_stmt ;

var_decl         = "bind" identifier ":" type "is" expression ";" ;

assignment       = identifier "is" expression ";" ;

emit_stmt        = "emit" expression ";" ;

if_stmt          = "check" "(" expression ")" block [ "otherwise" block ] ;

loop_stmt        = "cycle" "(" expression ")" block ;

function_decl    = "forge" identifier "(" [ parameter_list ] ")"
				   [ ":" type ] block ;

parameter_list   = parameter { "," parameter } ;
parameter        = identifier ":" type ;

return_stmt      = "yield" expression ";" ;

block            = "{" { statement } "}" ;

type             = "int" | "bool" ;

expression       = equality ;

equality         = comparison { ( "==" | "!=" ) comparison } ;
comparison       = term { ( ">" | "<" | ">=" | "<=" ) term } ;
term             = factor { ( "+" | "-" ) factor } ;
factor           = primary { ( "*" | "/" ) primary } ;
primary          = int_literal
				 | bool_literal
				 | identifier
				 | "(" expression ")" ;
```

## 5. Operator Precedence and Associativity

From lowest precedence to highest:

1. Equality: `==`, `!=`
2. Comparison: `>`, `<`, `>=`, `<=`
3. Additive: `+`, `-`
4. Multiplicative: `*`, `/`
5. Primary: literals, identifiers, parenthesized expressions

Binary operators are parsed left-to-right within each precedence level.

Examples:

1. `2 + 3 * 4` parses as `2 + (3 * 4)`.
2. `(2 + 3) * 4` uses explicit grouping.

## 6. Type System

### 6.1 Primitive Types

LiveLy currently supports two primitive types:

1. `int`
2. `bool`

### 6.2 Strict Static Typing Rules

1. Declarations must match initializer type.
2. Assignments must match the declared variable type.
3. No implicit conversion exists between `int` and `bool`.
4. Conditions for `check` and `cycle` must be `bool`.
5. Arithmetic operators require `int` operands and produce `int`.
6. Comparison operators require `int` operands and produce `bool`.
7. Equality operators require both operands to have the same type and produce `bool`.

Examples:

1. `bind x:int is 10;` is valid.
2. `bind x:int is alive;` is invalid.
3. `check (x >= 5) { ... }` is valid if `x` is `int`.
4. `check (x) { ... }` is invalid when `x` is `int`.

## 7. Declarations, Scope, and Name Resolution

### 7.1 Variables

Variables are introduced with `bind`:

```lively
bind x:int is 10;
```

Re-declaration in the same scope is rejected.

### 7.2 Lookup

Identifiers are resolved through nested scopes from innermost to outermost.

Using an undeclared variable is a semantic error.

### 7.3 Scope Behavior in v0.3

1. A global scope exists for top-level declarations.
2. Function bodies create a new scope.
3. Function parameters are declared in the function scope.
4. `check`/`otherwise`/`cycle` blocks are semantically analyzed in the current scope (no additional block scope layer yet).

## 8. Functions and Returns

### 8.1 Function Declarations

Syntax:

```lively
forge name(param:type, ...): return_type {
	...
}
```

Return type annotation is optional in syntax; if omitted, parser records `void`.

### 8.2 Return Checking

`yield` must produce a value whose type matches the function return type currently being analyzed.

Example:

```lively
forge clamp(n:int, limit:int): int {
	check (n <= limit) {
		yield n;
	} otherwise {
		yield limit;
	}
}
```

## 9. Statements

### 9.1 Variable Declaration

```lively
bind x:int is 10;
```

### 9.2 Assignment

```lively
x is x - 1;
```

### 9.3 Output

```lively
emit x;
```

### 9.4 Conditional

```lively
check (x >= 5) {
	emit x;
} otherwise {
	emit 0;
}
```

### 9.5 Loop

```lively
cycle (x != 0) {
	x is x - 1;
}
```

### 9.6 Return

```lively
yield x;
```

## 10. Error Model

### 10.1 Lexer Errors

The lexer reports errors for unknown characters/operators/symbols.

### 10.2 Parser Errors

The parser reports malformed syntax (missing separators, wrong token order, incomplete statements) with line-number context.

### 10.3 Semantic Errors

Semantic checks raise errors for:

1. Undeclared variables
2. Re-declarations in the same scope
3. Type mismatch in declaration/assignment
4. Invalid condition type in `check`/`cycle`
5. Invalid operand types for operators
6. Return type mismatch

## 11. Known Current Limitations (v0.4)

These are intentionally documented as current-state behavior:

1. No string/float/array/object types.
2. No unary operators (`-x`, logical `!x`) in grammar.
3. No logical boolean operators (`&&`, `||`) in grammar.
4. No function call expressions yet.
5. No comment syntax in lexer.
6. No separate lexical token for `IS` in active parsing flow (`is` is tokenized as assignment).
7. Return-path completeness checks (for example, ensuring all branches return) are not yet enforced.
8. `FunctionDecl` and `ReturnStmt` are not yet lowered to IR/Bytecode (skipped with a warning).
9. No `CALL`/`RET` opcodes in bytecode — function execution is not yet supported.

## 12. Minimal Complete Example

```lively
bind x:int is 10;
bind y:int is 2 + 3 * (4 - 1);
bind aliveflag:bool is alive;

x is x / y;

check (x >= 5) {
	emit x;
} otherwise {
	emit 0;
}

cycle (x != 0) {
	x is x - 1;
}

forge clamp(n:int, limit:int): int {
	check (n <= limit) {
		yield n;
	} otherwise {
		yield limit;
	}
}
```

This program is valid under the current LiveLy lexer, parser, semantic analyzer, TAC generator, and bytecode generator implementation. Note: the `forge` function declaration passes semantic analysis but is skipped during IR/Bytecode generation with a warning.