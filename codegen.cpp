#include "codegen.hpp"
#include <iostream>

void generateIntermediateCode(ASTNode* ast, vector<string>& code) {
    int tempCount = 0;
    if (ast) ast->generateIntermediateCode(code, tempCount);
}

void generateAssembly(ASTNode* ast, vector<string>& asmCode) {
    int regCount = 0;
    vector<pair<string, string>> stringLiterals;
    vector<string> instructions;
    
    if (ast) ast->generateAssembly(instructions, stringLiterals, regCount, "");
    
    // 1. Data Section
    asmCode.push_back("section .data");
    for (const auto& sl : stringLiterals) {
        // Handle escape characters if necessary, but simple for now
        asmCode.push_back("    " + sl.first + " db \"" + sl.second + "\", 0");
    }
    asmCode.push_back("");

    // 2. Text Section
    asmCode.push_back("section .text");
    asmCode.push_back("    global _main");
    asmCode.push_back("    extern _printf");
    asmCode.push_back("");
    
    // 3. Instructions
    for (const auto& instr : instructions) {
         // Indent instructions, but labels should be at start
         if (instr.back() == ':') {
             asmCode.push_back(instr);
         } else {
             asmCode.push_back("    " + instr);
         }
    }
} 