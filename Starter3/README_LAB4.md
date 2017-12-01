# Lab4 Doc
## Work Distribution
Chenlei Hu 1002030651   
handle_math_expr
handle_imm_val
handle_function
handle_constructor
handle_var_expr
Rongzhen Cui 1000221823  
handle_declaration
handle_assignment
handle_branch


## Problem encountered
### variable with same name in different scopes
It’s possible to have variable with same name used in different scopes, while the usage and declaration might not be in the same scope. We can not use symbol table to retrieve the definition since symbol table gets released dynamically in semantic check procedure. Our solution is adding an scope_id field to each variable node. It gets assigned in semantic check, and keep the information of associated scope of each variable. In codegen, we use the following structure `[OriginalVarName]_[ScopeId]` to represent each variable in different scopes.

## Novolties
### convert enum to string in C using preprocessor
```
#define FOREACH_INST(INST) \
    INST(TEMP)             \
    INST(ABS)              \
    INST(ADD)              \
    INST(CMP)              \
    INST(COS)              \
    INST(DP3)              
   ……

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum { FOREACH_INST(GENERATE_ENUM) } inst_code;
static const char* INST_STRING[] = {FOREACH_INST(GENERATE_STRING)};
```
Here we apply the C preprocessor to define an INST_STRING array so that for each instruction defined in FOREACH_INST, they will appear as enum member in `inst_code` type and string constant in INST_STRING array which can be accessed using its corresponding enum as array index. This makes each instruction taking single line of code instead of 2, also easier for adding new instructions later.

### Automatic testing
we also automated running demo tests with a perl script `test.pl` which makes verifying code result much easier.

## Non-trival math operators
```
                case '/':
                    // take reciprocate of opr2 and multiply with
                    // opr1
                    append_inst(RCP, out, opr2, "", "");
                    append_inst(MUL, out, opr1, out, "");
                    // if both operand is int type, convert result to int
                    if (is_in_set(INT_TYPES,
                                  ast->binary_expr.right->type_code) &&
                        is_in_set(INT_TYPES,
                                  ast->binary_expr.left->type_code)) {
                        append_inst(FLR, out, out, "", "");
                    }
                    break;
                case GEQ:
                    // opr1 >= opr2
                    // !(opr1 < opr2)
                    // !(opr1 - opr2 < 0)
                    append_inst(SUB, out, opr1, opr2, "");
                    append_inst(CMP, out, out, BOOL_FALSE, BOOL_TRUE);
                    break;
                case LEQ:
                    // opr1 <= opr2
                    // !(opr1 > opr2)
                    // !(opr1 - opr2 > 0)
                    // !(opr2 - opr1 < 0)
                    append_inst(SUB, out, opr2, opr1, "");
                    append_inst(CMP, out, out, BOOL_FALSE, BOOL_TRUE);
                    break;
                case '<':
                    // opr1 < opr2
                    // opr1 - opr2 < 0
                    append_inst(SUB, out, opr1, opr2, "");
                    append_inst(CMP, out, out, BOOL_TRUE, BOOL_FALSE);
                    break;
                case '>':
                    // opr1 > opr2
                    // opr2 - opr1 < 0
                    append_inst(SUB, out, opr2, opr1, "");
                    append_inst(CMP, out, out, BOOL_TRUE, BOOL_FALSE);
                    break;
                case EQ: {
                    // (opr1 >= opr2) && (opr2 <= opr1)
                    char* temp = assign_temp_reg();
                    append_inst(SUB, out, opr1, opr2, "");
                    append_inst(SUB, temp, opr2, opr1, "");
                    append_inst(CMP, out, out, BOOL_FALSE, BOOL_TRUE);
                    append_inst(CMP, temp, temp, BOOL_FALSE, BOOL_TRUE);
                    append_inst(MUL, out, temp, out, "");
                    free(temp);
                    break;
                }
                case NEQ: {
                    // (opr1 < opr2) || (opr1 > opr2)
                    char* temp = assign_temp_reg();
                    append_inst(SUB, out, opr1, opr2, "");
                    append_inst(SUB, temp, opr2, opr1, "");
                    append_inst(CMP, out, out, BOOL_TRUE, BOOL_FALSE);
                    append_inst(CMP, temp, temp, BOOL_TRUE, BOOL_FALSE);
                    append_inst(ADD, out, out, temp, "");
                    free(temp);
                    break;
                }
                case AND:
                    // opr1 * opr2
                    append_inst(MUL, out, opr1, opr2, "");
                    break;
                case OR:
                    // opr2 + opr1 - (opr1 * opr2)
                    append_inst(MUL, out, opr1, opr2, "");
                    append_inst(ADD, out, opr1, out, "");
                    append_inst(ADD, out, opr2, out, "");
                    break;
```
Related code is referenced here, most non-trivial math(logic) operators are mapped to a set of equivalent arb instructions.


## handling boolean types
```
#define ZERO_VEC "{0.0, 0.0, 0.0, 0.0}"
#define ONE_VEC "{1.0, 1.0, 1.0, 1.0}"

#define BOOL_FALSE ZERO_VEC
#define BOOL_TRUE ONE_VEC
```
Similar to boolean type definition in C, we define scalar int 1 as boolean true, and scalar int 0 as boolean fault. 

## how did you implement if statement
```
if (condition1) {
    var1 = x;
    var2 = y;
} else {
    if (condition2) {
        var2 = w;
    }
    var 1 = x;
}
```
Though we do not have branch nor jump in arb, we have the C ternary operator equivalent in arb which is CMP instruction. 
```
var1 = condition1 ? x: var1;
var2 = condition2 ? y: var2;

var2 = (!condition1) && condition2 ? w : var2;
var1 = !condition1 ? x: var1;
```

Modify the travesal function for if_node in following way:  
Compute the value of condition_node  
Compute the real condition value for the nested if statement by multiply outer condition and current condition, store it into a reg, say reg_condition  
Traversal the if_statements node, pass the reg_condition’s name and generate code on them, all assignment will be handled as above  
Compute the real condition value for the the else condition  
Traversal the else_statements node and do the same thing  



## how did you implement constant
```
switch (ast->kind) {
        case INT_NODE: {
            float val = (float)ast->literal_expr.int_val;
            snprintf(literal_expr, MAX_INS_LEN, "{%.1f, %.1f, %.1f, %.1f}", val,
                     val, val, val);
            append_inst(MOV, ast->reg_name, literal_expr, "", "");
            break;
        }
        case FLOAT_NODE: {
            float val = ast->literal_expr.float_val;
            snprintf(literal_expr, MAX_INS_LEN, "{%f, %f, %f, %f}", val,
                     val, val, val);
            append_inst(MOV, ast->reg_name, literal_expr, "", "");
            break;
        }
        case BOOL_NODE:
            if (ast->literal_expr.int_val == 1) {
                append_inst(MOV, ast->reg_name, BOOL_TRUE, "", "");
            } else if (ast->literal_expr.int_val == 0) {
                append_inst(MOV, ast->reg_name, BOOL_FALSE, "", "");
            } else {
                assert(0);
            }
            break;
        default:
            return;
    }
```
immediate values(constants) is handled as their scalar representation in arb expression respectively. 

## how the code for each type of node was generated  
Math expression Node (Unary, Binary): See above  
Declaration Node: append_inst(TEMP, reg_name, "", "", ""), reg_name is passed from variable node   
Assignment Node: 
inside if:         
append_inst(SUB, temp, BOOL_FALSE, ast->condi_reg_name, "");    append_inst(CMP, des_var, temp, src_var, des_var);  
outside if:
append_inst(MOV, des_var, src_var, "", "");
des_var, src_var are all register_name stored in child node  
Constructor Node: recursively visit child node of arguments and assign ‘.x’,’.y’,’.z’,’.w’ separately


