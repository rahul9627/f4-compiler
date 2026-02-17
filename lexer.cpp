#include "lexer.hpp"
#include <regex>
#include <cctype>

void tokenize(const string& source, vector<Token>& tokens, vector<string>& errors) {
    vector<pair<string, TokenType>> tokenSpecs = {
        {"int", TokenType::INT},
        {"return", TokenType::RETURN},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"==", TokenType::COMPARE},
        {"!=", TokenType::COMPARE},
        {"<=", TokenType::COMPARE},
        {">=", TokenType::COMPARE},
        {"<", TokenType::COMPARE},
        {">", TokenType::COMPARE},
        {"=", TokenType::ASSIGN},
        {"[+*/\-]", TokenType::OP},
        {"\\(", TokenType::LPAREN},
        {"\\)", TokenType::RPAREN},
        {"\\{", TokenType::LBRACE},
        {"\\}", TokenType::RBRACE},
        {";", TokenType::SEMI},
        {",", TokenType::COMMA},
        {"[0-9]+", TokenType::NUMBER},
        {"[a-zA-Z_][a-zA-Z0-9_]*", TokenType::ID},
    };
    size_t pos = 0;
    int line = 1;
    int lineStart = 0;
    while (pos < source.length()) {
        // Skip whitespace
        while (pos < source.length() && isspace(source[pos])) {
            if (source[pos] == '\n') {
                line++;
                lineStart = pos + 1;
            }
            pos++;
        }
        if (pos >= source.length()) break;
        // Skip preprocessor directives
        if (source[pos] == '#') {
            while (pos < source.length() && source[pos] != '\n') pos++;
            continue;
        }
        // Skip single-line comments
        if (source.substr(pos, 2) == "//") {
            pos += 2;
            while (pos < source.length() && source[pos] != '\n') pos++;
            continue;
        }
        // Skip multi-line comments
        if (source.substr(pos, 2) == "/*") {
            pos += 2;
            while (pos + 1 < source.length() && !(source[pos] == '*' && source[pos + 1] == '/')) {
                if (source[pos] == '\n') {
                    line++;
                    lineStart = pos + 1;
                }
                pos++;
            }
            if (pos + 1 < source.length()) pos += 2; // skip closing */
            continue;
        }
        // String literals
        if (source[pos] == '"') {
            size_t start = pos;
            pos++;
            while (pos < source.length()) {
                if (source[pos] == '\\' && pos + 1 < source.length()) {
                    pos += 2; // skip escaped char
                } else if (source[pos] == '"') {
                    pos++;
                    break;
                } else {
                    if (source[pos] == '\n') {
                        line++;
                        lineStart = pos + 1;
                    }
                    pos++;
                }
            }
            // Strip quotes: extract content between quotes
            string fullValue = source.substr(start, pos - start);
            string cleanValue = fullValue.substr(1, fullValue.length() - 2); // Remove first and last char (quotes)
            int column = start - lineStart;
            tokens.emplace_back(TokenType::STRING, cleanValue, line, column);
            continue;
        }
        bool matched = false;
        for (size_t i = 0; i < tokenSpecs.size(); ++i) {
            const string& pattern = tokenSpecs[i].first;
            TokenType type = tokenSpecs[i].second;
            regex re("^" + pattern);
            smatch match;
            string remaining = source.substr(pos);
            if (regex_search(remaining, match, re)) {
                string value = match.str();
                int column = pos - lineStart;
                tokens.emplace_back(type, value, line, column);
                pos += value.length();
                matched = true;
                break;
            }
        }
        if (!matched) {
            int column = pos - lineStart;
            errors.push_back("Illegal character '" + string(1, source[pos]) + "' at line " + to_string(line) + ", column " + to_string(column));
            pos++;
        }
    }
    tokens.emplace_back(TokenType::END, "", line, 0);
} 