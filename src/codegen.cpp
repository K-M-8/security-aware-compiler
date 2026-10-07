#include "codegen.h"
using namespace std;
static string pad(int n) { return string(n, ' '); }
string CodeGenerator::expr(const ExprPtr& e) const { return e->str(); }
void CodeGenerator::stmt(const StmtPtr& s, string& out, int indent) const {
    if (auto d = dynamic_pointer_cast<Declaration>(s)) { out += pad(indent) + labelToString(d->label) + " int " + d->name + ";\n"; return; }
    if (auto a = dynamic_pointer_cast<Assignment>(s)) { out += pad(indent) + a->name + " = " + expr(a->expr) + ";\n"; return; }
    if (auto p = dynamic_pointer_cast<PrintStmt>(s)) { out += pad(indent) + "print(" + expr(p->expr) + ");\n"; return; }
    if (auto i = dynamic_pointer_cast<IfStmt>(s)) {
        out += pad(indent) + "if (" + expr(i->condition) + ") {\n";
        for (auto& x : i->thenBlock->statements) stmt(x,out,indent+4);
        out += pad(indent) + "} else {\n";
        for (auto& x : i->elseBlock->statements) stmt(x,out,indent+4);
        out += pad(indent) + "}\n";
    }
}
string CodeGenerator::generate(const Program& p) const {
    string out;
    for (auto& s : p.statements) stmt(s,out,0);
    return out;
}
string CodeGenerator::report(const vector<Finding>& findings) const {
    string out = "========================================\nSECURITY ANALYSIS REPORT\n========================================\n\n";
    if (findings.empty()) { out += "No supported security leaks detected.\n"; return out; }
    for (size_t i=0;i<findings.size();++i) {
        const auto& f=findings[i];
        out += "Finding " + to_string(i+1) + "\n";
        out += "Vulnerability: " + f.type + "\n";
        out += "Severity: " + f.severity + "\n";
        out += "Source: " + f.source + "\n";
        out += "Sink: " + f.sink + "\n";
        out += "Information Flow: " + f.path + "\n";
        out += "Suggestion: " + f.suggestion + "\n\n";
    }
    return out;
}
