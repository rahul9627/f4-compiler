#include "parser.hpp"
#include "lexer.hpp"
#include <iostream>
#include <algorithm>

// Implement ASTNode print and printJSON methods here 
void ASTNode::print() const {
    cout << string(indentLevel * 2, ' ') << "└─ " << nodeType;
    if (!value.empty()) cout << " (" << value << ")";
    cout << endl;
    for (const auto& child : children) {
        child->print();
    }
}
void ASTNode::printJSON(ostream& out, int indent) const {
    string ind(indent, ' ');
    out << ind << "{\n";
    out << ind << "  \"type\": \"" << nodeType << "\"";
    if (!value.empty()) out << ",\n" << ind << "  \"value\": \"" << value << "\"";
    if (!children.empty()) {
        out << ",\n" << ind << "  \"children\": [\n";
        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->printJSON(out, indent + 4);
            if (i + 1 < children.size()) out << ",\n";
        }
        out << "\n" << ind << "  ]";
    }
    out << "\n" << ind << "}";
}
string ASTNode::generateIntermediateCode(vector<string>& code, int& tempCount) {
    if (nodeType == "Program") {
        for (const auto& child : children) {
            child->generateIntermediateCode(code, tempCount);
        }
    } else if (nodeType == "FunctionDecl") {
         code.push_back("func " + value);
         for (const auto& child : children) {
             child->generateIntermediateCode(code, tempCount);
         }
         code.push_back("endfunc");
    } else if (nodeType == "Block") {
        for (const auto& child : children) {
            child->generateIntermediateCode(code, tempCount);
        }
    } else if (nodeType == "Declaration") {
        if (!children.empty()) {
            string temp = children[0]->generateIntermediateCode(code, tempCount);
            code.push_back(value + " = " + temp);
        } else {
            code.push_back(value + " = 0");
        }
    } else if (nodeType == "Assignment") {
        if (!children.empty()) {
            string temp = children[0]->generateIntermediateCode(code, tempCount);
            code.push_back(value + " = " + temp);
        }
    } else if (nodeType == "BinaryExpr") {
        string leftTemp = children[0]->generateIntermediateCode(code, tempCount);
        string rightTemp = children[1]->generateIntermediateCode(code, tempCount);
        string resultTemp = "T" + to_string(++tempCount);
        code.push_back(resultTemp + " = " + leftTemp + " " + value + " " + rightTemp);
        return resultTemp;
    } else if (nodeType == "Identifier") {
        return value;
    } else if (nodeType == "NumberLiteral") {
        return value;
    } else if (nodeType == "StringLiteral") {
        return "\"" + value + "\"";
    } else if (nodeType == "FunctionCall") {
        // Generate param instructions for each argument
        for (const auto& child : children) {
            string arg = child->generateIntermediateCode(code, tempCount);
            code.push_back("param " + arg);
        }
        string resultTemp = "T" + to_string(++tempCount);
        code.push_back(resultTemp + " = call " + value + ", " + to_string(children.size()));
        return resultTemp;
    } else if (nodeType == "FunctionCallStatement") {
        string args;
        for (size_t i = 0; i < children.size(); ++i) {
            if (i > 0) args += ", ";
            args += children[i]->generateIntermediateCode(code, tempCount);
        }
        string resultTemp = "T" + to_string(++tempCount);
        code.push_back(resultTemp + " = call " + value + ", " + to_string(children.size()));
        return resultTemp;
    } else if (nodeType == "Return") {
        if (!children.empty()) {
            string retVal = children[0]->generateIntermediateCode(code, tempCount);
            code.push_back("return " + retVal);
        }
    } else if (nodeType == "IfElse") {
        string cond = children[0]->generateIntermediateCode(code, tempCount);
        string labelElse = "L" + to_string(++tempCount);
        string labelEnd = "L" + to_string(++tempCount);
        code.push_back("ifnot " + cond + " goto " + labelElse);
        children[1]->generateIntermediateCode(code, tempCount);
        code.push_back("goto " + labelEnd);
        code.push_back(labelElse + ":");
        if (children.size() > 2) {
            children[2]->generateIntermediateCode(code, tempCount);
        }
        code.push_back(labelEnd + ":");
    }
    return "";
}

string ASTNode::getRegister(int idx) const {
    static vector<string> regs = {"eax", "ebx", "ecx", "edx", "esi", "edi"};
    return regs[idx % regs.size()];
}

string ASTNode::generateAssembly(vector<string>& asmCode, vector<pair<string, string>>& stringLiterals, int& regCount, const string& currentFunc) const {
    if (nodeType == "NumberLiteral") {
        return value; // Return immediate value
    }
    if (nodeType == "Identifier") {
         // Should load variable value into a register for operation? 
         // For minimalism, assuming stack based variables or direct memory (requires formatting like [var])
         // But here we'll just return the identifier assuming the assembler handles 'mov eax, var'
         return "[" + value + "]";
    }
    if (nodeType == "StringLiteral") {
        string label = "LC" + to_string(stringLiterals.size());
        stringLiterals.push_back({label, value});
        return label;  // NASM syntax: just the label name
    }

    if (nodeType == "BinaryExpr") {
        // Simple register allocator: always use eax for result
        string leftOp = children[0]->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
        // If leftOp is not a register (e.g. memory or immediate), move it to eax
        asmCode.push_back("mov eax, " + leftOp);
        
        // We need to save eax if we evaluate right side? 
        // Simple compiler: push eax
        asmCode.push_back("push eax");
        
        string rightOp = children[1]->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
        
        asmCode.push_back("mov ebx, " + rightOp); // right operand in ebx
        asmCode.push_back("pop eax"); // left operand back in eax
        
        if (value == "+") {
            asmCode.push_back("add eax, ebx");
        } else if (value == "-") {
            asmCode.push_back("sub eax, ebx");
        } else if (value == "*") {
            asmCode.push_back("imul eax, ebx");
        } else if (value == "/") {
            asmCode.push_back("cdq");
            asmCode.push_back("idiv ebx");
        }
        return "eax";
    }

    if (nodeType == "Declaration" || nodeType == "Assignment") {
        if (!children.empty()) {
            string dim = children[0]->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
            // Result is in dim (likely eax if expression, or immediate)
             if (dim != "eax") {
                 asmCode.push_back("mov eax, " + dim);
             }
            // Assume variables are global or static for simplicity in this toy compiler
            asmCode.push_back("mov [" + value + "], eax");
        } else {
            asmCode.push_back("mov dword [" + value + "], 0");
        }
        return "";
    }

    if (nodeType == "FunctionCall") {
        // Cdecl convention: push args in reverse order
        size_t argCount = children.size();
        for (int i = argCount - 1; i >= 0; --i) {
            string arg = children[i]->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
            if (arg != "eax") {
                 asmCode.push_back("mov eax, " + arg);
            }
            asmCode.push_back("push eax");
        }
        string funcName = value;
        if (funcName == "printf") funcName = "_printf"; // decorate for Windows
        asmCode.push_back("call " + funcName);
        if (argCount > 0) {
            asmCode.push_back("add esp, " + to_string(argCount * 4));
        }
        return "eax"; // Result in eax
    }

    if (nodeType == "Program") {
        for (const auto& child : children) {
            child->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
        }
        return "";
    }

    if (nodeType == "FunctionDecl") {
        // Prologue
        string funcName = value;
        if (funcName == "main") funcName = "_main";
        
        asmCode.push_back(funcName + ":");
        asmCode.push_back("push ebp");
        asmCode.push_back("mov ebp, esp");
        
        // Create exit label name
        string exitLabel = ".Lexit_" + funcName;
        
        for (const auto& child : children) {
            child->generateAssembly(asmCode, stringLiterals, regCount, funcName);
        }
        
        // Epilogue (in case no return stmt) acts as target for jumps
        asmCode.push_back(exitLabel + ":");
        asmCode.push_back("mov esp, ebp");
        asmCode.push_back("pop ebp");
        asmCode.push_back("ret");
        return "";
    }
    
    if (nodeType == "Block") {
         for (const auto& child : children) {
            child->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
        }
        return "";
    }

    if (nodeType == "Return") {
        if (!children.empty()) {
            string retVal = children[0]->generateAssembly(asmCode, stringLiterals, regCount, currentFunc);
            if (retVal != "eax") {
                asmCode.push_back("mov eax, " + retVal);
            }
        }
        // Jump to shared epilogue
        string funcName = currentFunc;
        if (funcName == "main") funcName = "_main";
        string exitLabel = ".Lexit_" + funcName;
        asmCode.push_back("jmp " + exitLabel);
        return "";
    }

    return "";
}

// --- Parser implementation for minimal C with function call support ---

// Forward declarations
shared_ptr<ASTNode> parseStatement(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors);
shared_ptr<ASTNode> parseExpression(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors, int minPrec = 0);
shared_ptr<ASTNode> parseFunctionCall(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors);

shared_ptr<ASTNode> parseProgram(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors) {
    auto program = make_shared<ASTNode>("Program");
    
    // Expect: int main() { ... }
    if (tokens[currentTokenIndex].type != TokenType::INT) {
        errors.push_back("Expected 'int' at start of program");
        return nullptr;
    }
    currentTokenIndex++; // skip 'int'
    if (tokens[currentTokenIndex].type != TokenType::ID || tokens[currentTokenIndex].value != "main") {
        errors.push_back("Expected 'main' after 'int'");
        return nullptr;
    }
    string funcName = tokens[currentTokenIndex].value;
    currentTokenIndex++; // skip 'main'
    
    auto funcDecl = make_shared<ASTNode>("FunctionDecl", funcName);
    
    if (tokens[currentTokenIndex].type != TokenType::LPAREN) {
        errors.push_back("Expected '(' after 'main'");
        return nullptr;
    }
    currentTokenIndex++; // skip '('
    if (tokens[currentTokenIndex].type != TokenType::RPAREN) {
        errors.push_back("Expected ')' after '(' in main");
        return nullptr;
    }
    currentTokenIndex++; // skip ')'
    if (tokens[currentTokenIndex].type != TokenType::LBRACE) {
        errors.push_back("Expected '{' at start of main body");
        return nullptr;
    }
    currentTokenIndex++; // skip '{'
    
    auto block = make_shared<ASTNode>("Block");
    
    while (tokens[currentTokenIndex].type != TokenType::RBRACE && tokens[currentTokenIndex].type != TokenType::END) {
        auto stmt = parseStatement(tokens, currentTokenIndex, errors);
        if (stmt) block->addChild(stmt);
    }
    if (tokens[currentTokenIndex].type != TokenType::RBRACE) {
        errors.push_back("Expected '}' at end of main body");
        return nullptr;
    }
    currentTokenIndex++; // skip '}'
    
    funcDecl->addChild(block);
    program->addChild(funcDecl);
    
    return program;
}

shared_ptr<ASTNode> parseStatement(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors) {
    if (tokens[currentTokenIndex].type == TokenType::INT) {
        // Declaration
        currentTokenIndex++; // skip 'int'
        if (tokens[currentTokenIndex].type != TokenType::ID) {
            errors.push_back("Expected identifier after 'int'");
            return nullptr;
        }
        string varName = tokens[currentTokenIndex].value;
        currentTokenIndex++; // skip ID
        shared_ptr<ASTNode> expr = nullptr;
        if (tokens[currentTokenIndex].type == TokenType::ASSIGN) {
            currentTokenIndex++; // skip '='
            expr = parseExpression(tokens, currentTokenIndex, errors);
            if (!expr) {
                errors.push_back("Invalid expression in declaration");
                return nullptr;
            }
        }
        if (tokens[currentTokenIndex].type != TokenType::SEMI) {
            errors.push_back("Expected ';' at end of declaration");
            return nullptr;
        }
        currentTokenIndex++; // skip ';'
        auto decl = make_shared<ASTNode>("Declaration", varName);
        if (expr) decl->addChild(expr);
        return decl;
    } else if (tokens[currentTokenIndex].type == TokenType::ID) {
        // Assignment or function call
        size_t lookahead = currentTokenIndex + 1;
        if (tokens[lookahead].type == TokenType::ASSIGN) {
            // Assignment
            string varName = tokens[currentTokenIndex].value;
            currentTokenIndex += 2; // skip ID and '='
            auto expr = parseExpression(tokens, currentTokenIndex, errors);
            if (!expr) {
                errors.push_back("Invalid expression in assignment");
                return nullptr;
            }
            if (tokens[currentTokenIndex].type != TokenType::SEMI) {
                errors.push_back("Expected ';' at end of assignment");
                return nullptr;
            }
            currentTokenIndex++; // skip ';'
            auto assign = make_shared<ASTNode>("Assignment", varName);
            assign->addChild(expr);
            return assign;
        } else if (tokens[lookahead].type == TokenType::LPAREN) {
            // Function call
            return parseFunctionCall(tokens, currentTokenIndex, errors);
        } else {
            errors.push_back("Unexpected statement or keyword '" + tokens[currentTokenIndex].value + "' at line " + std::to_string(tokens[currentTokenIndex].line) + ", column " + std::to_string(tokens[currentTokenIndex].column));
            currentTokenIndex++;
            return nullptr;
        }
    } else if (tokens[currentTokenIndex].type == TokenType::IF) {
        // If/else
        currentTokenIndex++; // skip 'if'
        if (tokens[currentTokenIndex].type != TokenType::LPAREN) {
            errors.push_back("Expected '(' after 'if'");
            return nullptr;
        }
        currentTokenIndex++; // skip '('
        auto condition = parseExpression(tokens, currentTokenIndex, errors);
        if (!condition) {
            errors.push_back("Invalid condition in if statement");
            return nullptr;
        }
        if (tokens[currentTokenIndex].type != TokenType::RPAREN) {
            errors.push_back("Expected ')' after if condition");
            return nullptr;
        }
        currentTokenIndex++; // skip ')'
        if (tokens[currentTokenIndex].type != TokenType::LBRACE) {
            errors.push_back("Expected '{' after if condition");
            return nullptr;
        }
        currentTokenIndex++; // skip '{'
        auto ifBlock = make_shared<ASTNode>("Block");
        while (tokens[currentTokenIndex].type != TokenType::RBRACE && tokens[currentTokenIndex].type != TokenType::END) {
            auto stmt = parseStatement(tokens, currentTokenIndex, errors);
            if (stmt) ifBlock->addChild(stmt);
        }
        if (tokens[currentTokenIndex].type != TokenType::RBRACE) {
            errors.push_back("Expected '}' at end of if block");
            return nullptr;
        }
        currentTokenIndex++; // skip '}'
        shared_ptr<ASTNode> elseBlock = nullptr;
        if (tokens[currentTokenIndex].type == TokenType::ELSE) {
            currentTokenIndex++; // skip 'else'
            if (tokens[currentTokenIndex].type != TokenType::LBRACE) {
                errors.push_back("Expected '{' after 'else'");
                return nullptr;
            }
            currentTokenIndex++; // skip '{'
            elseBlock = make_shared<ASTNode>("Block");
            while (tokens[currentTokenIndex].type != TokenType::RBRACE && tokens[currentTokenIndex].type != TokenType::END) {
                auto stmt = parseStatement(tokens, currentTokenIndex, errors);
                if (stmt) elseBlock->addChild(stmt);
            }
            if (tokens[currentTokenIndex].type != TokenType::RBRACE) {
                errors.push_back("Expected '}' at end of else block");
                return nullptr;
            }
            currentTokenIndex++; // skip '}'
        }
        auto ifElseNode = make_shared<ASTNode>("IfElse");
        ifElseNode->addChild(condition);
        ifElseNode->addChild(ifBlock);
        if (elseBlock) ifElseNode->addChild(elseBlock);
        return ifElseNode;
    } else if (tokens[currentTokenIndex].type == TokenType::RETURN) {
        // Return
        currentTokenIndex++; // skip 'return'
        auto expr = parseExpression(tokens, currentTokenIndex, errors);
        if (!expr) {
            errors.push_back("Invalid expression in return statement");
            return nullptr;
        }
        if (tokens[currentTokenIndex].type != TokenType::SEMI) {
            errors.push_back("Expected ';' after return statement");
            return nullptr;
        }
        currentTokenIndex++; // skip ';'
        auto retNode = make_shared<ASTNode>("Return");
        retNode->addChild(expr);
        return retNode;
    } else {
        errors.push_back("Unexpected statement or keyword '" + tokens[currentTokenIndex].value + "' at line " + std::to_string(tokens[currentTokenIndex].line) + ", column " + std::to_string(tokens[currentTokenIndex].column));
        currentTokenIndex++;
        return nullptr;
    }
}

shared_ptr<ASTNode> parseFunctionCall(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors) {
    string funcName = tokens[currentTokenIndex].value;
    currentTokenIndex++; // skip ID
    if (tokens[currentTokenIndex].type != TokenType::LPAREN) {
        errors.push_back("Expected '(' after function name '" + funcName + "'");
        return nullptr;
    }
    currentTokenIndex++; // skip '('
    auto funcCall = make_shared<ASTNode>("FunctionCall", funcName);
    // Parse arguments (comma-separated expressions)
    bool first = true;
    while (tokens[currentTokenIndex].type != TokenType::RPAREN && tokens[currentTokenIndex].type != TokenType::END) {
        if (!first) {
            if (tokens[currentTokenIndex].type == TokenType::COMMA) {
                currentTokenIndex++; // skip ','
            } else {
                errors.push_back("Expected ',' between function arguments");
                return nullptr;
            }
        }
        auto arg = parseExpression(tokens, currentTokenIndex, errors);
        if (!arg) return nullptr;
        funcCall->addChild(arg);
        first = false;
    }
    if (tokens[currentTokenIndex].type != TokenType::RPAREN) {
        errors.push_back("Expected ')' after function arguments");
        return nullptr;
    }
    currentTokenIndex++; // skip ')'
    if (tokens[currentTokenIndex].type != TokenType::SEMI) {
        errors.push_back("Expected ';' after function call");
        return nullptr;
    }
    currentTokenIndex++; // skip ';'
    return funcCall;
}

shared_ptr<ASTNode> parsePrimary(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors) {
    if (tokens[currentTokenIndex].type == TokenType::ID) {
        auto node = make_shared<ASTNode>("Identifier", tokens[currentTokenIndex].value);
        currentTokenIndex++;
        return node;
    } else if (tokens[currentTokenIndex].type == TokenType::NUMBER) {
        auto node = make_shared<ASTNode>("NumberLiteral", tokens[currentTokenIndex].value);
        currentTokenIndex++;
        return node;
    } else if (tokens[currentTokenIndex].type == TokenType::STRING) {
        auto node = make_shared<ASTNode>("StringLiteral", tokens[currentTokenIndex].value);
        currentTokenIndex++;
        return node;
    } else {
        errors.push_back("Expected identifier, number, or string in expression");
        return nullptr;
    }
}

int getPrecedence(const string& op) {
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") return 0;
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/") return 2;
    return -1;
}

shared_ptr<ASTNode> parseExpression(const vector<Token>& tokens, size_t& currentTokenIndex, vector<string>& errors, int minPrec) {
    auto left = parsePrimary(tokens, currentTokenIndex, errors);
    while (true) {
        string op = tokens[currentTokenIndex].value;
        int prec = getPrecedence(op);
        if ((tokens[currentTokenIndex].type == TokenType::OP || tokens[currentTokenIndex].type == TokenType::COMPARE) && prec >= minPrec) {
            currentTokenIndex++;
            auto right = parseExpression(tokens, currentTokenIndex, errors, prec + 1);
            if (!right) return nullptr;
            auto bin = make_shared<ASTNode>("BinaryExpr", op);
            bin->addChild(left);
            bin->addChild(right);
            left = bin;
        } else {
            break;
        }
    }
    return left;
}
 