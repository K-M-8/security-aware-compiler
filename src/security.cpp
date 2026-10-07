#include "security.h"
using namespace std;

string labelToString(Label l) { return l == Label::SECRET ? "SECRET" : "PUBLIC"; }

Label SecurityAnalyzer::exprLabel(const ExprPtr& e, string& path) {
    if (auto v = dynamic_pointer_cast<VarExpr>(e)) {
        auto it = symbols.find(v->name);
        Label l = it == symbols.end() ? Label::PUBLIC : it->second;
        if (l == Label::SECRET) path = v->name + " [SECRET]";
        return l;
    }
    if (dynamic_pointer_cast<IntExpr>(e)) return Label::PUBLIC;
    auto b = dynamic_pointer_cast<BinaryExpr>(e);
    string lp, rp;
    Label l = exprLabel(b->left, lp), r = exprLabel(b->right, rp);
    if (l == Label::SECRET || r == Label::SECRET) {
        path = (lp.empty() ? b->left->str() : lp) + " -> " + b->op + " -> " + (rp.empty() ? b->right->str() : rp);
        return Label::SECRET;
    }
    return Label::PUBLIC;
}

void SecurityAnalyzer::analyzeBlock(const shared_ptr<Block>& b) {
    for (const auto& s : b->statements) analyzeStmt(s);
}

void SecurityAnalyzer::analyzeStmt(const StmtPtr& s) {
    if (auto d = dynamic_pointer_cast<Declaration>(s)) { symbols[d->name] = d->label; return; }
    if (auto a = dynamic_pointer_cast<Assignment>(s)) {
        string path; Label l = exprLabel(a->expr, path);
        if (symbols.count(a->name) && symbols[a->name] == Label::SECRET) l = Label::SECRET;
        symbols[a->name] = l;
        return;
    }
    if (auto p = dynamic_pointer_cast<PrintStmt>(s)) {
        string path; Label l = exprLabel(p->expr, path);
        if (l == Label::SECRET) findings.push_back({"Information-flow leak", "HIGH", path, "public output print()", path + " -> print()", "Do not expose SECRET data through public output.", 0});
        return;
    }
    if (auto i = dynamic_pointer_cast<IfStmt>(s)) {
        string path; Label l = exprLabel(i->condition, path);
        if (l == Label::SECRET) findings.push_back({"Secret-dependent branch", "HIGH", path, "if condition", path + " -> branch", "Replace supported if-else assignments with constant-time select.", 0});
        analyzeBlock(i->thenBlock);
        analyzeBlock(i->elseBlock);
    }
}

vector<Finding> SecurityAnalyzer::analyze(const Program& p) {
    symbols.clear(); findings.clear();
    for (const auto& s : p.statements) analyzeStmt(s);
    return findings;
}
