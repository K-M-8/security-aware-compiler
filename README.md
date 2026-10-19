# Security-Aware Compiler for Detecting and Rewriting Timing-Leak Code

A small C-like compiler prototype based on the Review-1 project. It implements a complete educational pipeline:

Source -> Lexer -> Parser/AST -> PUBLIC/SECRET information-flow analysis -> TAC -> CFG -> leak detection -> restricted transformation -> security report.

## Language

Supported declarations:

```text
secret int key;
public int x;
int y;
```

Supported statements:

```text
x = key + 10;
print(x);
if (key > 100) { x = 10; } else { x = 20; }
```

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Or directly with g++:

```bash
g++ -std=c++17 src/*.cpp -o security_compiler
```

## Run

From the project root:

```bash
./security_compiler examples/timing_leak.sac
```

The compiler prints the symbol table, security findings, TAC, CFG blocks, and supported transformations. It also writes files into `output/`.

## Project mapping

- Lexer: `src/lexer.*`
- Parser + AST: `src/parser.*`, `src/ast.h`
- Information-flow analysis and leak detection: `src/security.*`
- TAC and CFG: `src/tac.*`
- Secure transformation: `src/transform.*`
- Code/report generation: `src/codegen.*`
- Driver: `src/main.cpp`

## Scope

This is intentionally a small C-like language, not a full C compiler. Automatic rewriting is restricted to an `if/else` where both branches contain one assignment to the same variable. The project uses a conceptual `ct_select(condition, trueValue, falseValue)` representation for the secure branchless form.


## Added in the information-flow improvement

The analyzer now keeps provenance for secret-derived variables, so it can explain flows through intermediate assignments (for example, `key -> temp -> result -> print`). It also conservatively marks assignments inside a secret-dependent `if` as secret-derived, then reports a later public-output sink.

New examples:
- `examples/transitive_flow.sac` — secret-derived value flows through multiple assignments before printing.
- `examples/implicit_flow.sac` — a secret-dependent branch controls a public variable's assignment.

These are static-analysis warnings, not proof that a program is exploitable. This remains a small educational language and does not implement a complete formal information-flow system. A `ct_select(...)` source rewrite is only a transformation candidate; it does not by itself prove constant-time machine code.
