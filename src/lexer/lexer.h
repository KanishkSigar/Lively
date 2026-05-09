#pragma once
#include <string>
#include <vector>
#include "token.h"

class Lexer
{
private:
    std::string source;
    size_t position;
    int line;
    int column;

public:
    Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:
    char currentChar() const;
    char peek() const;
    void advance();

    void skipWhitespace();

    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readOperator();
    Token readSymbol();

    bool isAtEnd() const;
};