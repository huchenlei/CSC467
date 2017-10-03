# Lab1 documentation
## Chenlei Hu
- Int cast func
- Float cast func
- Other 1to1 mapping token parse func(s)
## RongZhen Cui
- Boolean cast func
- Identifier cast func

## Problems encountered
1. When detecting integer, patterns like "123abc" will be considered as a INT token and an IDENTIFIER token, which is not what we expected(it should be considered an invalid token). By inspecting flex documentation(http://people.cs.aau.dk/~marius/sw/flex/Flex-Regular-Expressions.html), we found that trailing context is useful here. Append "/{DELIM}" at the end of original express solves the problem. "123abc" will no longer be accepted as 123 since it's not followed by a delim(while space, etc).

2. We first considered "\n" as part of whitespace([WS]), but in this the scanner will not count lines for error reporting. By considering "\n" as a special case with {yyline++;} as corresponding action, the error report can now correctly report line number with unrecognized token.

