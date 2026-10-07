#pragma once
#include <memory>
#include <string>
#include <vector>

using namespace std;

enum class Label { PUBLIC, SECRET };

string labelToString(Label l);

struct Expr {
    virtual ~Expr() = default;
    virtual string str() const = 0;
};
using ExprPtr = shared_ptr<Expr>;

struct IntExpr : Expr {
    int value;
    explicit IntExpr(int v) : value(v) {}
    string str() const override { return to_string(value); }
};

struct VarExpr : Expr {
    string name;
    explicit VarExpr(string n) : name(move(n)) {}
    string str() const override { return name; }
};

struct BinaryExpr : Expr {
    string op;
    ExprPtr left, right;
    BinaryExpr(string o, ExprPtr l, ExprPtr r) : op(move(o)), left(move(l)), right(move(r)) {}
    string str() const override { return "(" + left->str() + " " + op + " " + right->str() + ")"; }
};

struct Statement {
    virtual ~Statement() = default;
};
using StmtPtr = shared_ptr<Statement>;

struct Declaration : Statement {
    Label label;
    string type;
    string name;
    Declaration(Label l, string t, string n) : label(l), type(move(t)), name(move(n)) {}
};

struct Assignment : Statement {
    string name;
    ExprPtr expr;
    Assignment(string n, ExprPtr e) : name(move(n)), expr(move(e)) {}
};

struct PrintStmt : Statement {
    ExprPtr expr;
    explicit PrintStmt(ExprPtr e) : expr(move(e)) {}
};

struct Block : Statement {
    vector<StmtPtr> statements;
};

struct IfStmt : Statement {
    ExprPtr condition;
    shared_ptr<Block> thenBlock;
    shared_ptr<Block> elseBlock;
    IfStmt(ExprPtr c, shared_ptr<Block> t, shared_ptr<Block> e) : condition(move(c)), thenBlock(move(t)), elseBlock(move(e)) {}
};

struct Program {
    vector<StmtPtr> statements;
};
