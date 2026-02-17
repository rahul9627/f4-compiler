#include "semantic.hpp"

// Helper to check if a block contains a return statement
bool hasReturnStatement(ASTNode* node) {
    if (!node) return false;
    if (node->nodeType == "Return") return true;
    for (const auto& child : node->children) {
        if (hasReturnStatement(child.get())) return true;
    }
    return false;
}

void semanticAnalysis(ASTNode* node, map<string, string>& symbolTable, vector<string>& errors) {
    if (!node) return;
    if (node->nodeType == "Program") {
        for (const auto& child : node->children) {
            semanticAnalysis(child.get(), symbolTable, errors);
        }
    } else if (node->nodeType == "FunctionDecl") {
        // Check if int function has return statement
        bool hasReturn = hasReturnStatement(node);
        if (!hasReturn) {
            errors.push_back("Function '" + node->value + "' with return type 'int' must have a return statement.");
        }
        // Analyze function body
        for (const auto& child : node->children) {
            semanticAnalysis(child.get(), symbolTable, errors);
        }
    } else if (node->nodeType == "Block") {
        for (const auto& child : node->children) {
            semanticAnalysis(child.get(), symbolTable, errors);
        }
    } else if (node->nodeType == "Declaration") {
        if (symbolTable.find(node->value) != symbolTable.end()) {
            errors.push_back("Variable '" + node->value + "' already declared.");
        } else {
            symbolTable[node->value] = "int";
        }
        if (!node->children.empty()) {
            semanticAnalysis(node->children[0].get(), symbolTable, errors);
        }
    } else if (node->nodeType == "Assignment") {
        if (symbolTable.find(node->value) == symbolTable.end()) {
            errors.push_back("Undeclared variable '" + node->value + "' in assignment.");
        }
        if (!node->children.empty()) {
            semanticAnalysis(node->children[0].get(), symbolTable, errors);
        }
    } else if (node->nodeType == "Identifier") {
        if (symbolTable.find(node->value) == symbolTable.end()) {
            errors.push_back("Undeclared variable '" + node->value + "'.");
        }
    } else if (node->nodeType == "BinaryExpr") {
        semanticAnalysis(node->children[0].get(), symbolTable, errors);
        semanticAnalysis(node->children[1].get(), symbolTable, errors);
    } else if (node->nodeType == "Return") {
        if (!node->children.empty())
            semanticAnalysis(node->children[0].get(), symbolTable, errors);
    } else if (node->nodeType == "IfElse") {
        for (const auto& child : node->children) {
            semanticAnalysis(child.get(), symbolTable, errors);
        }
    } else if (node->nodeType == "FunctionCall") {
        // Basic check: just analyze arguments
        for (const auto& child : node->children) {
            semanticAnalysis(child.get(), symbolTable, errors);
        }
    }
}