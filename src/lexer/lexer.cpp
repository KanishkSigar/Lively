#include "lexer.h"
#include <cctype>
#include <unordered_map>
#include <stdexcept>

// Keyword Mapping
static std::unordered_map<std::string, TokenType> keywords = {
    {"bind", TokenType::BIND},
    {"is", TokenType::ASSIGN},
    {"emit", TokenType::EMIT},
    {"check", TokenType::CHECK},
    {"otherwise", TokenType::OTHERWISE},
    {"cycle", TokenType::CYCLE},
    {"forge", TokenType::FORGE},
    {"yield", TokenType::YIELD},
    {"alive", TokenType::BOOL_LITERAL},
    {"dead", TokenType::BOOL_LITERAL},
    {"int", TokenType::TYPE_INT},
    {"bool", TokenType::TYPE_BOOL}
};

// Constructor
Lexer::Lexer(const std::string& source)
    : source(source), position(0), line(1), column(1) {}

// Core Functions
bool Lexer::isAtEnd() const {
    return position >= source.length();
}

char Lexer::currentChar() const {
    return isAtEnd() ? '\0' : source[position];
}

char Lexer::peek() const {
    return (position + 1 < source.length()) ? source[position + 1] : '\0';
}

void Lexer::advance() {
    if (currentChar() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    position++;
}

// Whitespace Handling
void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(currentChar())) {
        advance();
    }
}

// ==============================
// Token Readers
// ==============================

Token Lexer::readIdentifierOrKeyword() {
    int startCol = column;
    std::string value;

    while (std::isalnum(currentChar())) {
        value += currentChar();
        advance();
    }

    if (keywords.count(value)) {
        return Token(keywords[value], value, line, startCol);
    }

    return Token(TokenType::IDENTIFIER, value, line, startCol);
}

Token Lexer::readNumber() {
    int startCol = column;
    std::string value;

    while (std::isdigit(currentChar())) {
        value += currentChar();
        advance();
    }

    return Token(TokenType::INT_LITERAL, value, line, startCol);
}

Token Lexer::readOperator() {
    int startCol = column;
    char c = currentChar();

    switch (c) {
        case '+': advance(); return Token(TokenType::PLUS, "+", line, startCol);
        case '-': advance(); return Token(TokenType::MINUS, "-", line, startCol);
        case '*': advance(); return Token(TokenType::MULTIPLY, "*", line, startCol);
        case '/': advance(); return Token(TokenType::DIVIDE, "/", line, startCol);

        case '>':
            if (peek() == '=') {
                advance(); advance();
                return Token(TokenType::GREATER_EQUAL, ">=", line, startCol);
            }
            advance();
            return Token(TokenType::GREATER, ">", line, startCol);

        case '<':
            if (peek() == '=') {
                advance(); advance();
                return Token(TokenType::LESS_EQUAL, "<=", line, startCol);
            }
            advance();
            return Token(TokenType::LESS, "<", line, startCol);

        case '=':
            if (peek() == '=') {
                advance(); advance();
                return Token(TokenType::EQUAL, "==", line, startCol);
            }
            break;

        case '!':
            if (peek() == '=') {
                advance(); advance();
                return Token(TokenType::NOT_EQUAL, "!=", line, startCol);
            }
            break;
    }

    throw std::runtime_error("Unknown operator at line " + std::to_string(line));
}

Token Lexer::readSymbol() {
    int startCol = column;
    char c = currentChar();

    switch (c) {
        case ':': advance(); return Token(TokenType::COLON, ":", line, startCol);
        case ';': advance(); return Token(TokenType::SEMICOLON, ";", line, startCol);
        case ',': advance(); return Token(TokenType::COMMA, ",", line, startCol);
        case '(': advance(); return Token(TokenType::LPAREN, "(", line, startCol);
        case ')': advance(); return Token(TokenType::RPAREN, ")", line, startCol);
        case '{': advance(); return Token(TokenType::LBRACE, "{", line, startCol);
        case '}': advance(); return Token(TokenType::RBRACE, "}", line, startCol);
    }

    throw std::runtime_error("Unknown symbol at line " + std::to_string(line));
}

// ==============================
// Main Tokenizer
// ==============================

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();

        if (isAtEnd()) break;

        char c = currentChar();

        if (std::isalpha(c)) {
            tokens.push_back(readIdentifierOrKeyword());
        }
        else if (std::isdigit(c)) {
            tokens.push_back(readNumber());
        }
        else if (std::ispunct(c)) {
            if (std::string("+-*/><=!").find(c) != std::string::npos) {
                tokens.push_back(readOperator());
            } else {
                tokens.push_back(readSymbol());
            }
        }
        else {
            throw std::runtime_error("Unexpected character at line " + std::to_string(line));
        }
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
    return tokens;
}