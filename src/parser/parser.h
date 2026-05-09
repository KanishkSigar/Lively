#pragma once
#include <vector>
#include <memory>
#include "../lexer/token.h"
#include "../ast/ast.h"

// Parser Class
class Parser {
private:
    std::vector<Token> tokens;
    size_t current;

public:
    Parser(const std::vector<Token>& tokens);

    // Entry point
    std::vector<std::unique_ptr<Statement>> parse();

private:
    // Core Navigation
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;

    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);

    Token consume(TokenType type, const std::string& errorMessage);

    // Statement Parsing
    std::unique_ptr<Statement> parseStatement();

    std::unique_ptr<Statement> parseVarDecl();
    std::unique_ptr<Statement> parseAssignment();
    std::unique_ptr<Statement> parseEmit();
    std::unique_ptr<Statement> parseIf();
    std::unique_ptr<Statement> parseLoop();
    std::unique_ptr<Statement> parseReturn();
    std::unique_ptr<Statement> parseFunction();

    std::vector<std::unique_ptr<Statement>> parseBlock();

    // Expression Parsing
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parsePrimary();
};