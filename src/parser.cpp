#include "parser.h"
#include <stdexcept>
using namespace std;

const Token& Parser::cur() const { return tokens[pos]; }
bool Parser::match(TokenType t) { if (cur().type == t) { ++pos; return true; } return false; }
Token Parser::expect(TokenType t, const string& what) { if (cur().type != t) throw runtime_error("Expected " + what + " at line " + to_string(cur().line)); return tokens[pos++]; }

Program Parser::parse() {
    Program p;
    while (cur().type != TokenType::END) p.statements.push_back(statement());
    return p;
}

StmtPtr Parser::statement() {
    if (cur().type == TokenType::SECRET || cur().type == TokenType::PUBLIC || cur().type == TokenType::INT) return declaration();
    if (cur().type == TokenType::IF) return ifStatement();
    if (cur().type == TokenType::PRINT) return printStatement();
    if (cur().type == TokenType::IDENT) return assignment();
    throw runtime_error("Unexpected token '" + cur().text + "' at line " + to_string(cur().line));
}

StmtPtr Parser::declaration() {
    Label label = Label::PUBLIC;
    if (match(TokenType::SECRET)) label = Label::SECRET;
    else match(TokenType::PUBLIC);
    expect(TokenType::INT, "int");
    string name = expect(TokenType::IDENT, "identifier").text;
    expect(TokenType::SEMICOLON, ";");
    return make_shared<Declaration>(label, "int", name);
}

StmtPtr Parser::assignment() {
    string name = expect(TokenType::IDENT, "identifier").text;
    expect(TokenType::EQ, "=");
    auto e = expression();
    expect(TokenType::SEMICOLON, ";");
    return make_shared<Assignment>(name, e);
}

StmtPtr Parser::printStatement() {
    expect(TokenType::PRINT, "print");
    expect(TokenType::LPAREN, "(");
    auto e = expression();
    expect(TokenType::RPAREN, ")");
    expect(TokenType::SEMICOLON, ";");
    return make_shared<PrintStmt>(e);
}

StmtPtr Parser::ifStatement() {
    expect(TokenType::IF, "if");
    expect(TokenType::LPAREN, "(");
    auto condition = expression();
    expect(TokenType::RPAREN, ")");
    auto thenBlock = block();
    shared_ptr<Block> elseBlock = make_shared<Block>();
    if (match(TokenType::ELSE)) elseBlock = block();
    return make_shared<IfStmt>(condition, thenBlock, elseBlock);
}

shared_ptr<Block> Parser::block() {
    auto b = make_shared<Block>();
    expect(TokenType::LBRACE, "{");
    while (cur().type != TokenType::RBRACE && cur().type != TokenType::END) b->statements.push_back(statement());
    expect(TokenType::RBRACE, "}");
    return b;
}

ExprPtr Parser::expression() { return equality(); }
ExprPtr Parser::equality() {
    auto e = comparison();
    while (cur().type == TokenType::EQEQ || cur().type == TokenType::NEQ) {
        string op = cur().text; ++pos; e = make_shared<BinaryExpr>(op, e, comparison());
    }
    return e;
}
ExprPtr Parser::comparison() {
    auto e = term();
    while (cur().type == TokenType::LT || cur().type == TokenType::GT || cur().type == TokenType::LE || cur().type == TokenType::GE) {
        string op = cur().text; ++pos; e = make_shared<BinaryExpr>(op, e, term());
    }
    return e;
}
ExprPtr Parser::term() {
    auto e = factor();
    while (cur().type == TokenType::PLUS || cur().type == TokenType::MINUS) {
        string op = cur().text; ++pos; e = make_shared<BinaryExpr>(op, e, factor());
    }
    return e;
}
ExprPtr Parser::factor() {
    auto e = primary();
    while (cur().type == TokenType::STAR || cur().type == TokenType::SLASH) {
        string op = cur().text; ++pos; e = make_shared<BinaryExpr>(op, e, primary());
    }
    return e;
}
ExprPtr Parser::primary() {
    if (cur().type == TokenType::NUMBER) return make_shared<IntExpr>(stoi(expect(TokenType::NUMBER, "number").text));
    if (cur().type == TokenType::IDENT) return make_shared<VarExpr>(expect(TokenType::IDENT, "identifier").text);
    if (match(TokenType::LPAREN)) { auto e = expression(); expect(TokenType::RPAREN, ")"); return e; }
    throw runtime_error("Expected expression at line " + to_string(cur().line));
}
