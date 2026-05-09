#include "parser.h"
#include <stdexcept>

// Constructor
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}
bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}
Token Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}
Token Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) {
        return advance();
    }

    throw std::runtime_error(
        errorMessage + " at line " + std::to_string(peek().line)
    );
}

//Entry Point
std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;

    while (!isAtEnd()) {
        statements.push_back(parseStatement());
    }

    return statements;
}

//Dispatcher
std::unique_ptr<Statement> Parser::parseStatement() {

    if (match(TokenType::BIND)) {
        return parseVarDecl();
    }
    if (match(TokenType::EMIT)) {
        return parseEmit();
    }
    if (match(TokenType::CHECK)) {
        return parseIf();
    }
    if (match(TokenType::CYCLE)) {
        return parseLoop();
    }
    if (match(TokenType::FORGE)) {
        return parseFunction();
    }
    if (match(TokenType::YIELD)) {
        return parseReturn();
    }
    // Fallback → Assignment
    if (check(TokenType::IDENTIFIER)) {
        return parseAssignment();
    }
    throw std::runtime_error(
        "Unexpected statement at line " + std::to_string(peek().line)
    );
}

//Variable Declaration
std::unique_ptr<Statement> Parser::parseVarDecl() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    consume(TokenType::COLON, "Expected ':' after variable name");

    Token typeToken = advance();
    if (typeToken.type != TokenType::TYPE_INT &&
        typeToken.type != TokenType::TYPE_BOOL) {
        throw std::runtime_error("Expected type after ':'");
    }

    consume(TokenType::ASSIGN, "Expected 'is' after type");

    std::unique_ptr<Expression> value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    return std::make_unique<VarDecl>(
        name.value,
        typeToken.value,
        std::move(value)
    );
}

//Assignment
std::unique_ptr<Statement> Parser::parseAssignment() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    consume(TokenType::ASSIGN, "Expected 'is' in assignment");

    std::unique_ptr<Expression> value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after assignment");

    return std::make_unique<Assignment>(
        name.value,
        std::move(value)
    );
}

//Emit Statement
std::unique_ptr<Statement> Parser::parseEmit() {
    // 'emit' already consumed

    std::unique_ptr<Expression> value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after emit statement");

    return std::make_unique<EmitStmt>(std::move(value));
}

//Block Parsing
std::vector<std::unique_ptr<Statement>> Parser::parseBlock() {
    std::vector<std::unique_ptr<Statement>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }

    consume(TokenType::RBRACE, "Expected '}' after block");

    return statements;
}

//If Statenent
std::unique_ptr<Statement> Parser::parseIf() {

    consume(TokenType::LPAREN, "Expected '(' after check");

    auto condition = parseExpression();

    consume(TokenType::RPAREN, "Expected ')' after condition");

    consume(TokenType::LBRACE, "Expected '{' before if block");

    auto thenBranch = parseBlock();

    std::vector<std::unique_ptr<Statement>> elseBranch;

    if (match(TokenType::OTHERWISE)) {
        consume(TokenType::LBRACE, "Expected '{' before else block");
        elseBranch = parseBlock();
    }

    auto stmt = std::make_unique<IfStmt>(std::move(condition));
    stmt->thenBranch = std::move(thenBranch);
    stmt->elseBranch = std::move(elseBranch);

    return stmt;
}

//Return Statement
std::unique_ptr<Statement> Parser::parseReturn() {

    auto value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after return");

    return std::make_unique<ReturnStmt>(std::move(value));
}

//Loop Statement
std::unique_ptr<Statement> Parser::parseLoop() {

    consume(TokenType::LPAREN, "Expected '(' after cycle");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after loop condition");

    consume(TokenType::LBRACE, "Expected '{' before loop body");
    auto body = parseBlock();

    auto stmt = std::make_unique<LoopStmt>(std::move(condition));
    stmt->body = std::move(body);
    return stmt;
}

//Function Declaration
std::unique_ptr<Statement> Parser::parseFunction() {

    Token name = consume(TokenType::IDENTIFIER, "Expected function name after forge");

    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<std::pair<std::string, std::string>> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
            consume(TokenType::COLON, "Expected ':' after parameter name");

            Token paramType = advance();
            if (paramType.type != TokenType::TYPE_INT &&
                paramType.type != TokenType::TYPE_BOOL) {
                throw std::runtime_error("Expected parameter type");
            }

            parameters.push_back({paramName.value, paramType.value});
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameter list");

    // Optional return type: forge name(...): int { ... }
    std::string returnType = "void";
    if (match(TokenType::COLON)) {
        Token typeToken = advance();
        if (typeToken.type != TokenType::TYPE_INT &&
            typeToken.type != TokenType::TYPE_BOOL) {
            throw std::runtime_error("Expected function return type");
        }
        returnType = typeToken.value;
    }

    consume(TokenType::LBRACE, "Expected '{' before function body");
    auto body = parseBlock();

    auto fn = std::make_unique<FunctionDecl>(name.value, returnType);
    fn->parameters = std::move(parameters);
    fn->body = std::move(body);
    return fn;
}

//Expression Entry
std::unique_ptr<Expression> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison();

    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
        Token op = previous();
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(op.value, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseTerm();

    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        Token op = previous();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(op.value, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(op.value, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseFactor() {
    auto expr = parsePrimary();

    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        Token op = previous();
        auto right = parsePrimary();
        expr = std::make_unique<BinaryExpr>(op.value, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    if (match(TokenType::INT_LITERAL) || match(TokenType::BOOL_LITERAL)) {
        return std::make_unique<LiteralExpr>(previous().value);
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpr>(previous().value);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    throw std::runtime_error(
        "Expected expression at line " + std::to_string(peek().line)
    );
}

