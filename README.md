# ptxc — PTX Compiler Frontend & Optimization Toolkit

ptxc is a C++17-based compiler frontend and optimization tool for a subset of NVIDIA PTX (GPU assembly). It implements a full compilation pipeline including lexical analysis, parsing, SSA-based IR, and classical compiler optimizations.

---

## 🚀 Features
- PTX lexer and recursive-descent parser
- SSA-based intermediate representation (IR)
- Control Flow Graph (CFG) construction
- Dominator tree (Lengauer–Tarjan algorithm)
- Optimization passes:
  - Dead Code Elimination (DCE)
  - Constant Folding
  - Common Subexpression Elimination (CSE)
- PTX code emitter with round-trip validation

---

## 🏗️ Build
```bash
mkdir build && cd build
cmake ..
make -j
