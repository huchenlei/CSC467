# Lab2 documentation
We used the Start Code for Lab2.
## Testing
Test the code with
```
perl ./test/<test_name>
```
---
## Chenlei Hu
- write the auto-testing script `parserTest.pl`
- implement main body of parser.y (50%)
## RongZheng Cui
- implement main body of parser.y (50%)
- add some test case
## Problems encountered
1. scanner.l does not recognize comma. we add `","    { yOUT(yytext[0]); }` into the scanner file
2. After implement all the production rule in the handout, we get a warnning
```
warning: rule useless in parser due to conflicts
| /*epsilon*/    {yTRACE("declaration -> epsilon")}
```
since we already have a rule:
`declarations -> declarations declaration | epsilon`, so we removed the `declaration -> epsilon` rule.
4. the precedence of unary_op '-' and binary '-' are different, need to add '%prec UMINUS` to specify the orecedence of unary_op '-'
5. the starting parser.y file does not have correct precedence and associativity rule, we reimplement it based on the handout.
## Something interesting
we get `warning: shift/reduce conflicts`, and find if we combine `expression->unary_op expression` and `unary_op->!|-` to `expression->!expression|-expression` will reduce the number of shift/reduce.   
