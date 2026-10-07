#include "lexer.h"
#include "parser.h"
#include "security.h"
#include "tac.h"
#include "transform.h"
#include "codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

static string readFile(const string& path) {
    ifstream f(path);
    if (!f) throw runtime_error("Cannot open input file: " + path);
    stringstream ss; ss << f.rdbuf(); return ss.str();
}
static void writeFile(const string& path, const string& text) { ofstream f(path); f << text; }

int main(int argc, char** argv) {
    if (argc != 2) { cerr << "Usage: ./security_compiler <source.sac>\n"; return 1; }
    try {
        string source = readFile(argv[1]);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        Program program = parser.parse();

        SecurityAnalyzer analyzer;
        auto findings = analyzer.analyze(program);
        IRGenerator ir;
        auto irResult = ir.generate(program);
        Transformer transformer;
        auto transformations = transformer.transform(program);
        CodeGenerator codegen;

        cout << "\n=== SECURITY-AWARE COMPILER ===\n";
        cout << "\n[1] Lexical analysis: PASS\n";
        cout << "[2] Parsing / AST construction: PASS\n";
        cout << "[3] Information-flow analysis: PASS\n";
        cout << "\nSymbol Table\n";
        for (const auto& [name,label] : analyzer.symbolTable()) cout << "  " << name << " -> " << labelToString(label) << "\n";
        cout << "\n" << codegen.report(findings);
        cout << "=== THREE ADDRESS CODE ===\n";
        for (auto& x : irResult.tac) cout << x.text << "\n";
        cout << "\n=== CFG BLOCKS ===\n";
        for (auto& b : irResult.cfg) {
            cout << "B" << b.id << ": ";
            for (auto& x : b.instructions) cout << "[" << x.text << "] ";
            cout << " -> ";
            for (auto s : b.successors) cout << "B" << s << " ";
            cout << "\n";
        }
        cout << "\n=== TRANSFORMATION ===\n";
        if (transformations.empty()) cout << "No supported automatic transformation found.\n";
        for (const auto& t : transformations) {
            cout << "BEFORE: " << t.before << "\n";
            cout << "AFTER:  " << t.after << "\n";
        }
        writeFile("output/security_report.txt", codegen.report(findings));
        writeFile("output/tac.txt", [&]{ string s; for(auto& x:irResult.tac) s += x.text + "\n"; return s; }());
        writeFile("output/transformed_code.sac", [&]{
            string s;
            for (auto& x : transformations) s += x.after + "\n";
            return s;
        }());
        cout << "\nGenerated files in output/: security_report.txt, tac.txt, transformed_code.sac\n";
    } catch (const exception& e) { cerr << "ERROR: " << e.what() << "\n"; return 1; }
}
