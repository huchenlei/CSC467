# CSC467 Compiler and Interpreters 2017 F
## Project Outline
### Phase 1 (Lab1 ~ 2)
#### Lab1
Writing a scanner(tokenizer) with flex. (Basically writing regular expressions)  
Most work done in `scanner.l`.
#### Lab2
Writing a parser with bison. (Writing unambiguous context-free grammar(CFG))  
Most work done in `parser.y`.

### Phase 2 (Lab3 ~ 4)
#### Lab3
Construction of Abstract Syntax Tree (AST), and doing semantic analysis.  
Most work done in `semantic.c`, `ast.c`, `symbol.c`.

#### Lab4
Code generation. Map each part of mini-GL language to assembly language.  
Most work done in `codegen.c`.

## Testing
Test the code with
```
perl ./test/<test_name>
```
