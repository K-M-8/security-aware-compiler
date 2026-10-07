#pragma once
#include "ast.h"
#include <string>
#include <vector>
using namespace std;

struct TACInstruction { string text; bool secret = false; };
struct BasicBlock { int id; vector<TACInstruction> instructions; vector<int> successors; };

struct IRResult { vector<TACInstruction> tac; vector<BasicBlock> cfg; };

class IRGenerator {
    int temp = 0, label = 0;
    vector<TACInstruction> code;
    string newTemp();
    string newLabel();
    string expr(const ExprPtr& e);
    void stmt(const StmtPtr& s);
public:
    IRResult generate(const Program& p);
};
