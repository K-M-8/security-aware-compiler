#pragma once
#include "ast.h"
#include "security.h"
#include <string>
#include <vector>
using namespace std;
class CodeGenerator {
    string expr(const ExprPtr& e) const;
    void stmt(const StmtPtr& s, string& out, int indent) const;
public:
    string generate(const Program& p) const;
    string report(const vector<Finding>& findings) const;
};
