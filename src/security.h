#pragma once
#include "ast.h"
#include <map>
#include <string>
#include <vector>
using namespace std;

struct Finding {
    string type;
    string severity;
    string source;
    string sink;
    string path;
    string suggestion;
    int lineHint;
};

class SecurityAnalyzer {
    map<string, Label> symbols;
    map<string, Label> declaredLabels;
    map<string, string> provenance;
    vector<string> controlDependencies;
    vector<Finding> findings;

    Label exprLabel(const ExprPtr& e, string& path);
    void analyzeStmt(const StmtPtr& s);
    void analyzeBlock(const shared_ptr<Block>& b);

public:
    vector<Finding> analyze(const Program& p);
    const map<string, Label>& symbolTable() const { return symbols; }
};
