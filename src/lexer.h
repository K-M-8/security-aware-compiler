#pragma once
#include <string>
#include <vector>
using namespace std;

enum class TokenType {
    END, IDENT, NUMBER, SECRET, PUBLIC, INT, IF, ELSE, PRINT,
    PLUS, MINUS, STAR, SLASH, EQ, EQEQ, NEQ, LT, GT, LE, GE,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA
};

struct Token {
    TokenType type;
    string text;
    int line;
    int column;
};

class Lexer {
    string source;
    size_t pos = 0;
    int line = 1, column = 1;
public:
    explicit Lexer(string s) : source(move(s)) {}
    vector<Token> tokenize();
};
