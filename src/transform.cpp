#include "transform.h"
using namespace std;
string Transformer::expr(const ExprPtr& e) const { return e->str(); }
vector<Transformation> Transformer::transform(Program& p) {
    vector<Transformation> out;
    for (auto& s : p.statements) {
        auto i = dynamic_pointer_cast<IfStmt>(s);
        if (!i || i->thenBlock->statements.size()!=1 || i->elseBlock->statements.size()!=1) continue;
        auto a = dynamic_pointer_cast<Assignment>(i->thenBlock->statements[0]);
        auto b = dynamic_pointer_cast<Assignment>(i->elseBlock->statements[0]);
        if (!a || !b || a->name != b->name) continue;
        string before = "if (" + expr(i->condition) + ") " + a->name + " = " + expr(a->expr) + "; else " + a->name + " = " + expr(b->expr) + ";";
        string after = a->name + " = ct_select(" + expr(i->condition) + ", " + expr(a->expr) + ", " + expr(b->expr) + ");";
        out.push_back({true,before,after});
    }
    return out;
}
