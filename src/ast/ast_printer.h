#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "ast.h"

// ============================================================
// AST Printer — Produces a visual syntax tree on the terminal.
// Each node prints itself with indentation to show nesting.
// ============================================================

class ASTPrinter {
public:
    // Print every top-level statement in the program.
    static void printProgram(const std::vector<std::unique_ptr<Statement>>& statements) {
        std::cout << "\n==============================\n";
        std::cout << "       ABSTRACT SYNTAX TREE   \n";
        std::cout << "==============================\n\n";

        for (size_t i = 0; i < statements.size(); ++i) {
            std::cout << "[" << i << "] ";
            printStatement(statements[i].get(), 0);
            std::cout << "\n";
        }

        std::cout << "==============================\n";
        std::cout << "       END OF AST             \n";
        std::cout << "==============================\n";
    }

private:
    // Helper: produce indentation string
    static std::string indent(int level) {
        std::string result;
        for (int i = 0; i < level; ++i) {
            if (i == level - 1)
                result += "  +-- ";
            else
                result += "  |   ";
        }
        return result;
    }

    // ---- Statement dispatchers ----

    static void printStatement(const Statement* stmt, int level) {
        if (auto* v = dynamic_cast<const VarDecl*>(stmt)) {
            printVarDecl(v, level);
        } else if (auto* a = dynamic_cast<const Assignment*>(stmt)) {
            printAssignment(a, level);
        } else if (auto* e = dynamic_cast<const EmitStmt*>(stmt)) {
            printEmit(e, level);
        } else if (auto* i = dynamic_cast<const IfStmt*>(stmt)) {
            printIf(i, level);
        } else if (auto* l = dynamic_cast<const LoopStmt*>(stmt)) {
            printLoop(l, level);
        } else if (auto* r = dynamic_cast<const ReturnStmt*>(stmt)) {
            printReturn(r, level);
        } else if (auto* f = dynamic_cast<const FunctionDecl*>(stmt)) {
            printFunction(f, level);
        } else {
            std::cout << indent(level) << "<unknown statement>\n";
        }
    }

    // ---- Concrete statement printers ----

    static void printVarDecl(const VarDecl* v, int level) {
        std::cout << indent(level) << "VarDecl: " << v->name
                  << " : " << v->type << "\n";
        std::cout << indent(level + 1) << "Value:\n";
        printExpression(v->value.get(), level + 2);
    }

    static void printAssignment(const Assignment* a, int level) {
        std::cout << indent(level) << "Assignment: " << a->name << "\n";
        std::cout << indent(level + 1) << "Value:\n";
        printExpression(a->value.get(), level + 2);
    }

    static void printEmit(const EmitStmt* e, int level) {
        std::cout << indent(level) << "Emit:\n";
        printExpression(e->value.get(), level + 1);
    }

    static void printIf(const IfStmt* i, int level) {
        std::cout << indent(level) << "IfStmt (check):\n";

        std::cout << indent(level + 1) << "Condition:\n";
        printExpression(i->condition.get(), level + 2);

        std::cout << indent(level + 1) << "Then (" << i->thenBranch.size()
                  << " stmt" << (i->thenBranch.size() != 1 ? "s" : "") << "):\n";
        for (const auto& s : i->thenBranch) {
            printStatement(s.get(), level + 2);
        }

        if (!i->elseBranch.empty()) {
            std::cout << indent(level + 1) << "Otherwise (" << i->elseBranch.size()
                      << " stmt" << (i->elseBranch.size() != 1 ? "s" : "") << "):\n";
            for (const auto& s : i->elseBranch) {
                printStatement(s.get(), level + 2);
            }
        }
    }

    static void printLoop(const LoopStmt* l, int level) {
        std::cout << indent(level) << "LoopStmt (cycle):\n";

        std::cout << indent(level + 1) << "Condition:\n";
        printExpression(l->condition.get(), level + 2);

        std::cout << indent(level + 1) << "Body (" << l->body.size()
                  << " stmt" << (l->body.size() != 1 ? "s" : "") << "):\n";
        for (const auto& s : l->body) {
            printStatement(s.get(), level + 2);
        }
    }

    static void printReturn(const ReturnStmt* r, int level) {
        std::cout << indent(level) << "ReturnStmt (yield):\n";
        printExpression(r->value.get(), level + 1);
    }

    static void printFunction(const FunctionDecl* f, int level) {
        std::cout << indent(level) << "FunctionDecl: " << f->name
                  << " -> " << f->returnType << "\n";

        if (!f->parameters.empty()) {
            std::cout << indent(level + 1) << "Params:\n";
            for (const auto& p : f->parameters) {
                std::cout << indent(level + 2) << p.first << " : " << p.second << "\n";
            }
        }

        std::cout << indent(level + 1) << "Body (" << f->body.size()
                  << " stmt" << (f->body.size() != 1 ? "s" : "") << "):\n";
        for (const auto& s : f->body) {
            printStatement(s.get(), level + 2);
        }
    }

    // ---- Expression dispatchers ----

    static void printExpression(const Expression* expr, int level) {
        if (auto* lit = dynamic_cast<const LiteralExpr*>(expr)) {
            std::cout << indent(level) << "Literal: " << lit->value << "\n";
        } else if (auto* var = dynamic_cast<const VariableExpr*>(expr)) {
            std::cout << indent(level) << "Variable: " << var->name << "\n";
        } else if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
            std::cout << indent(level) << "BinaryOp: " << bin->op << "\n";
            std::cout << indent(level + 1) << "Left:\n";
            printExpression(bin->left.get(), level + 2);
            std::cout << indent(level + 1) << "Right:\n";
            printExpression(bin->right.get(), level + 2);
        } else {
            std::cout << indent(level) << "<unknown expression>\n";
        }
    }
};
