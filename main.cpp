#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "codegen.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename.c>" << endl;
        return 1;
    }
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "Error: Could not open file '" << argv[1] << "'" << endl;
        return 1;
    }
    string sourceCode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    cout << "=== Source Code ===" << endl;
    cout << sourceCode << endl << endl;

    // Phase 1: Lexical Analysis
    cout << "=== Lexical Analysis (Tokenization) ===" << endl;
    vector<Token> tokens;
    vector<string> errors;
    tokenize(sourceCode, tokens, errors);
    for (const auto& token : tokens) {
        cout << "Line " << token.line << ", Column " << token.column << ": ";
        switch (token.type) {
            case TokenType::INT: cout << "Keyword (int)"; break;
            case TokenType::RETURN: cout << "Keyword (return)"; break;
            case TokenType::IF: cout << "Keyword (if)"; break;
            case TokenType::ELSE: cout << "Keyword (else)"; break;
            case TokenType::ID: cout << "ID"; break;
            case TokenType::NUMBER: cout << "NUMBER"; break;
            case TokenType::STRING: cout << "STRING"; break;
            case TokenType::OP: cout << "OP"; break;
            case TokenType::COMPARE: cout << "COMPARE"; break;
            case TokenType::ASSIGN: cout << "ASSIGN"; break;
            case TokenType::LPAREN: cout << "LPAREN"; break;
            case TokenType::RPAREN: cout << "RPAREN"; break;
            case TokenType::LBRACE: cout << "LBRACE"; break;
            case TokenType::RBRACE: cout << "RBRACE"; break;
            case TokenType::SEMI: cout << "SEMI"; break;
            case TokenType::COMMA: cout << "COMMA"; break;
            case TokenType::END: cout << "END"; break;
        }
        cout << " = " << token.value << endl;
    }
    if (!errors.empty()) {
        cout << "\nCompilation errors:" << endl;
        for (const auto& error : errors) {
            cout << error << endl;
        }
        return 1;
    }

    // Phase 2: Syntax Analysis (Parsing)
    cout << "\n=== Syntax Analysis (Parsing) ===" << endl;
    size_t currentTokenIndex = 0;
    shared_ptr<ASTNode> ast = parseProgram(tokens, currentTokenIndex, errors);
    if (!errors.empty()) {
        cout << "\nCompilation errors:" << endl;
        for (const auto& error : errors) {
            cout << error << endl;
        }
        return 1;
    }
    cout << "\nParse Tree (JSON):" << endl;
    if (ast) ast->printJSON(cout, 0);
    cout << endl;

    // Phase 3: Semantic Analysis
    cout << "\n=== Semantic Analysis ===" << endl;
    map<string, string> symbolTable;
    semanticAnalysis(ast.get(), symbolTable, errors);
    if (!errors.empty()) {
        cout << "\nCompilation errors:" << endl;
        for (const auto& error : errors) {
            cout << error << endl;
        }
        return 1;
    }
    cout << "Semantic analysis passed!" << endl;

    // Phase 4: Intermediate Code Generation
    cout << "\n=== Intermediate Code Generation ===" << endl;
    vector<string> intermediateCode;
    generateIntermediateCode(ast.get(), intermediateCode);
    cout << "\nIntermediate Code (Three-Address Code):" << endl;
    for (size_t i = 0; i < intermediateCode.size(); ++i) {
        cout << i << ": " << intermediateCode[i] << endl;
    }

    // Phase 5: Assembly Code Generation
    cout << "\n=== Assembly Code Generation ===" << endl;
    vector<string> asmCode;
    generateAssembly(ast.get(), asmCode);
    cout << "\nAssembly Code:" << endl;
    for (size_t i = 0; i < asmCode.size(); ++i) {
        cout << i << ": " << asmCode[i] << endl;
    }
    return 0;
} 