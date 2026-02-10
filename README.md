# LLVM IR Parser and Analyzer for Per-Function JIT Pre-test

## Build (macOS):
```
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
clang++ -o parse_module pretest_2.cpp \
    $(llvm-config --cxxflags --ldflags --libs core irreader) -lLLVM
```

## Usage:
  ./parse_module pretest_2.ll