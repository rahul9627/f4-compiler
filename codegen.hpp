#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <vector>
#include <string>
#include "parser.hpp"
using namespace std;

void generateIntermediateCode(ASTNode* ast, vector<string>& code);
void generateAssembly(ASTNode* ast, vector<string>& asmCode);

#endif // CODEGEN_HPP 