# Lab3
## Bouns:
- const qualified variables must be initialized with constant expressions.
- report the line number on which the semantic error occurred
- Ensure that every variable has been assigned a value before being read.
- Provide a contextual message that gives extra meaning to the error to help the programmer debug the error:
	- if the error involves writing to a const qualified variable, then report the line number where that variable was declared
	- if double declare a variable, then report the line number where that variable was declared
	- report type or variable name for some errors

## approach to type/semantic checking:
### General rules in expression eval
#### operator
- if either operands is not CONST, the result is not labeled CONST;
- write only variable can not be operand;

## approach to AST structure:

---

## breakdown of work:
### RONGZHEN CUI (1000221823):
	- construct AST(ast.h, ast.c)
    - semantic check
      - ast_function_check
      - ast_argument_check
      - ast_constructor_check
      - ast_simple_expr_eval
### CHENLEI HU (1002030651):
	- construct symbol table(symbol.h, symbol.c)
    - semantic check
      - ast_operator_check
      - ast_condition_check
      - ast_assignment_check
      - ast_declaration_check
      - ast_variable_check

---
## novelties
### symbol table
We use a linked list to store variables declared in a scope, but we do NOT malloc
memory for each variable in symbol table, since malloc is an 'expensive' function
when the number of declarations is massive. Instead, we allocate a baseline number
of entries at start, and each time we need a new entry we just either advance the
stack pointer in this big block of memory, or we allocate another big block of memory
when the previous stack is full.

---
## challenges faced
### symbol table
Because we use multiple stacks to store entries, the tearing down of symbol table
becomes an issue. The solution is relatively simple: we add a private field `_is_pivot`
to each entry to track whether the entry is represent the whole stack, and only free
those entries when tearing down.
