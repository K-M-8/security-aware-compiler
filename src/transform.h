#pragma once
#include "ast.h"
#include <vector>
#include <string>
using namespace std;
struct Transformation { bool applied=false; string before; string after; };
class Transformer {
    string expr(const ExprPtr& e) const;
public:
    vector<Transformation> transform(Program& p);
};
