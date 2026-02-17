#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
using namespace std;

enum class TokenType {
    INT, RETURN, IF, ELSE,
    ID, NUMBER, STRING,
    OP, COMPARE, ASSIGN,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMI,
    COMMA,
    END
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    Token(TokenType t, string v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};

void tokenize(const string& source, vector<Token>& tokens, vector<string>& errors);

#endif // LEXER_HPP 