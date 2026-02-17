#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <map>
#include <string>
#include <vector>
#include "parser.hpp"
using namespace std;

void semanticAnalysis(ASTNode* node, map<string, string>& symbolTable, vector<string>& errors);

#endif // SEMANTIC_HPP 