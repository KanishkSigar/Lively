#pragma once
#include <string>

// Token Types Enumeration
enum class TokenType
{
    // Keywords
    BIND,
    IS,
    EMIT,
    CHECK,
    OTHERWISE,
    CYCLE,
    FORGE,
    YIELD,

    // Types
    TYPE_INT,
    TYPE_BOOL,

    // Literals
    INT_LITERAL,
    BOOL_LITERAL,

    // Identifier
    IDENTIFIER,

    // Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    GREATER,        // >
    LESS,           // <
    GREATER_EQUAL,  // >=
    LESS_EQUAL,     // <=
    EQUAL,          // ==
    NOT_EQUAL,      // !=

    // Assignment
    ASSIGN,         // is

    // Symbols
    COLON,          // :
    SEMICOLON,      // ;
    COMMA,          // ,

    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }

    // Special
    END_OF_FILE
};

// Token Structure
struct Token
{
    TokenType type;
    std::string value;

    int line;
    int column;

    Token(TokenType type, std::string value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
};