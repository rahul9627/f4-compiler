#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include <memory>
using namespace std;

enum class TokenType;
struct Token;

struct ASTNode {
    string nodeType;
    vector<shared_ptr<ASTNode>> children;
    string value;
    int indentLevel = 0;
    ASTNode(string type, string val = "") : nodeType(type), value(val) {}
    void addChild(shared_ptr<ASTNode> child) {
        child->indentLevel = indentLevel + 1;
        children.push_back(child);
    }
    void print() const;
    void printJSON(ostream& out, int indent = 0) const;
    string generateIntermediateCode(vector<string>& code, int& tempCount);
    string getRegister(int idx) const;
    string generateAssembly(vector<string>& asmCode, vector<pair<string, string>>& stringLiterals, int& regCount, const string& currentFunc = "") const;
};

shared_ptr<ASTNode> parseProgram(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors);

#endif // PARSER_HPP 