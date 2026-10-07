#include "tac.h"
using namespace std;
string IRGenerator::newTemp() { return "t" + to_string(++temp); }
string IRGenerator::newLabel() { return "L" + to_string(++label); }
string IRGenerator::expr(const ExprPtr& e) {
    if (auto i = dynamic_pointer_cast<IntExpr>(e)) return to_string(i->value);
    if (auto v = dynamic_pointer_cast<VarExpr>(e)) return v->name;
    auto b = dynamic_pointer_cast<BinaryExpr>(e);
    string l = expr(b->left), r = expr(b->right), t = newTemp();
    code.push_back({t + " = " + l + " " + b->op + " " + r, false});
    return t;
}
void IRGenerator::stmt(const StmtPtr& s) {
    if (auto d = dynamic_pointer_cast<Declaration>(s)) return;
    if (auto a = dynamic_pointer_cast<Assignment>(s)) { code.push_back({a->name + " = " + expr(a->expr), false}); return; }
    if (auto p = dynamic_pointer_cast<PrintStmt>(s)) { code.push_back({"print " + expr(p->expr), false}); return; }
    if (auto i = dynamic_pointer_cast<IfStmt>(s)) {
        string cond = expr(i->condition), t = newLabel(), e = newLabel(), end = newLabel();
        code.push_back({"if " + cond + " goto " + t, false});
        code.push_back({"goto " + e, false});
        code.push_back({t + ":", false});
        for (auto& x : i->thenBlock->statements) stmt(x);
        code.push_back({"goto " + end, false});
        code.push_back({e + ":", false});
        for (auto& x : i->elseBlock->statements) stmt(x);
        code.push_back({end + ":", false});
    }
}
IRResult IRGenerator::generate(const Program& p) {
    code.clear(); temp = label = 0;
    for (auto& s : p.statements) stmt(s);
    vector<BasicBlock> blocks;
    int id = 0;
    BasicBlock b{id++};
    for (auto& ins : code) {
        b.instructions.push_back(ins);
        if (!ins.text.empty() && ins.text.back() == ':') { if (b.instructions.size() > 1) { blocks.push_back(b); b = BasicBlock{id++}; } }
    }
    if (!b.instructions.empty()) blocks.push_back(b);
    for (size_t k=0; k+1<blocks.size(); ++k) blocks[k].successors.push_back(blocks[k+1].id);
    return {code, blocks};
}
