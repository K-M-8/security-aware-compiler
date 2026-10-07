#pragma once
#include "ast.h"
#include "lexer.h"
#include <vector>
class Parser {
    vector<Token> tokens;
    size_t pos = 0;
    const Token& cur() const;
    bool match(TokenType t);
    Token expect(TokenType t, const string& what);
    StmtPtr statement();
    StmtPtr declaration();
    StmtPtr assignment();
    StmtPtr printStatement();
    StmtPtr ifStatement();
    shared_ptr<Block> block();
    ExprPtr expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr primary();
public:
    explicit Parser(vector<Token> t) : tokens(move(t)) {}
    Program parse();
};
