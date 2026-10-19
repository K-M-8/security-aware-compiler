#include "security.h"
#include <algorithm>
using namespace std;

string labelToString(Label l) {
    return l == Label::SECRET ? "SECRET" : "PUBLIC";
}

static string joinPath(const string& a, const string& b, const string& op = " -> ") {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a == b) return a;
    return a + op + b;
}

Label SecurityAnalyzer::exprLabel(const ExprPtr& e, string& path) {
    if (!e) return Label::PUBLIC;

    if (auto v = dynamic_pointer_cast<VarExpr>(e)) {
        auto it = symbols.find(v->name);
        Label label = it == symbols.end() ? Label::PUBLIC : it->second;
        if (label == Label::SECRET) {
            auto p = provenance.find(v->name);
            path = (p != provenance.end() && !p->second.empty())
                ? p->second
                : v->name + " [SECRET]";
        }
        return label;
    }

    if (dynamic_pointer_cast<IntExpr>(e)) return Label::PUBLIC;

    auto b = dynamic_pointer_cast<BinaryExpr>(e);
    if (!b) return Label::PUBLIC;

    string leftPath, rightPath;
    Label leftLabel = exprLabel(b->left, leftPath);
    Label rightLabel = exprLabel(b->right, rightPath);

    if (leftLabel == Label::SECRET || rightLabel == Label::SECRET) {
        string left = leftLabel == Label::SECRET ? leftPath : b->left->str();
        string right = rightLabel == Label::SECRET ? rightPath : b->right->str();
        path = left + " -> " + b->op + " -> " + right;
        return Label::SECRET;
    }
    return Label::PUBLIC;
}

void SecurityAnalyzer::analyzeBlock(const shared_ptr<Block>& b) {
    if (!b) return;
    for (const auto& s : b->statements) analyzeStmt(s);
}

void SecurityAnalyzer::analyzeStmt(const StmtPtr& s) {
    if (auto d = dynamic_pointer_cast<Declaration>(s)) {
        symbols[d->name] = d->label;
        declaredLabels[d->name] = d->label;
        if (d->label == Label::SECRET)
            provenance[d->name] = d->name + " [SECRET]";
        else
            provenance.erase(d->name);
        return;
    }

    if (auto a = dynamic_pointer_cast<Assignment>(s)) {
        string expressionPath;
        Label expressionLabel = exprLabel(a->expr, expressionPath);

        bool declaredSecret = declaredLabels.count(a->name) && declaredLabels[a->name] == Label::SECRET;
        bool secretControl = !controlDependencies.empty();
        if (expressionLabel == Label::SECRET || declaredSecret || secretControl) {
            symbols[a->name] = Label::SECRET;
            string newPath = expressionLabel == Label::SECRET
                ? expressionPath
                : (declaredSecret ? a->name + " [SECRET]" : "");
            for (const auto& dependency : controlDependencies)
                newPath = joinPath(newPath, dependency);
            newPath = joinPath(newPath, "assignment " + a->name);
            provenance[a->name] = newPath;
        } else {
            symbols[a->name] = Label::PUBLIC;
            provenance.erase(a->name);
        }
        return;
    }

    if (auto p = dynamic_pointer_cast<PrintStmt>(s)) {
        string path;
        Label label = exprLabel(p->expr, path);
        if (label == Label::SECRET) {
            findings.push_back({
                "Information-flow leak", "HIGH", path, "public output print()",
                path + " -> print()", "Do not expose SECRET-derived data through public output.", 0
            });
        }
        return;
    }

    if (auto i = dynamic_pointer_cast<IfStmt>(s)) {
        string conditionPath;
        Label conditionLabel = exprLabel(i->condition, conditionPath);
        if (conditionLabel == Label::SECRET) {
            findings.push_back({
                "Secret-dependent branch", "HIGH", conditionPath, "if condition",
                conditionPath + " -> branch", 
                "Replace only supported if-else assignments with a validated constant-time selection.", 0
            });
            controlDependencies.push_back(conditionPath + " -> control dependency");
        }

        analyzeBlock(i->thenBlock);
        analyzeBlock(i->elseBlock);

        if (conditionLabel == Label::SECRET)
            controlDependencies.pop_back();
    }
}

vector<Finding> SecurityAnalyzer::analyze(const Program& p) {
    symbols.clear();
    declaredLabels.clear();
    provenance.clear();
    controlDependencies.clear();
    findings.clear();

    for (const auto& s : p.statements) analyzeStmt(s);
    return findings;
}
