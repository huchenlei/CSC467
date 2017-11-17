#include <assert.h>
#include "ast.h"
#include "common.h"
#include "parser.tab.h"
#include "semantic.h"

void ast_pre_check(node* ast, int depth);
void ast_post_check(node* ast, int depth);

int semantic_check(node* ast) {
    ast_visit(ast, 0, &ast_pre_check, &ast_post_check);
    return 0;
}

static const int logic_ops[3] = {OR, AND, '!'};
static const int arithmetic_ops[6] = {'+', '-', '*', '/', '^', UMINUS};
static const int comparison_ops[6] = {EQ, NEQ, '<', LEQ, '>', GEQ};
static const int boolean_types[4] = {FALSE_C, TRUE_C, BOOL_T, BVEC_T};
static const int scala_boolean_types[3] = {TRUE_C, FALSE_C, BOOL_T};
static const int vec_boolean_types[1] = {BVEC_T};
static const int arithmetic_types[6] = {VEC_T,  FLOAT_T, FLOAT_C,
                                        IVEC_T, INT_T,   INT_C};
static const int scala_arithmetic_types[4] = {FLOAT_T, FLOAT_C, INT_T, INT_C};
static const int vec_arithmetic_types[2] = {VEC_T, IVEC_T};

int is_in_set(const int* arr, size_t len, int target) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target) {
            return 1;  // true
        }
    }
    return 0;  // false
}

void ast_operator_check(node* ast) {
    node_kind kind = ast->kind;
    int op = 0;
    node* oprands[2];
    size_t oplen = 0;

    switch (kind) {
        case BINARY_EXPRESSION_NODE:
            op = ast->binary_expr.op;
            oplen = 2;
            oprands[0] = ast->binary_expr.left;
            oprands[1] = ast->binary_expr.right;
            break;
        case UNARY_EXPRESION_NODE:
            op = ast->unary_expr.op;
            oplen = 1;
            oprands[0] = ast->unary_expr.right;
            break;
        default:
            // Do nothing for other nodes
            return;
    }

    // operands with invalid types generally is rejected
    for (size_t i = 0; i < oplen; i++) {
        if (oprands[i]->type_code == -1 || oprands[i]->vec_size == 0) {
            fprintf(errorFile, "%d: type error due to previous errors\n",
                    ast->line);
            goto ast_operator_check_error;
        }
    }

    // if nothing is wrong sv, vs, vv, ss will all yeild result type with the
    // operands with highest order
    if (oplen == 1 || oprands[0]->vec_size > oprands[1]->vec_size) {
        ast->vec_size = oprands[0]->vec_size;
        ast->type_code = oprands[0]->type_code;
    } else {
        ast->vec_size = oprands[1]->vec_size;
        ast->type_code = oprands[1]->type_code;
    }

    if (is_in_set(logic_ops, 3, op)) {
        // All operands to logical ops must have boolean types
        for (size_t i = 0; i < oplen; i++) {
            if (!is_in_set(boolean_types, 4, oprands[i]->type_code)) {
                fprintf(errorFile,
                        "%d: %s operator must have bool type as %ld operand\n",
                        ast->line, get_binary_op_str(op), i + 1);
                goto ast_operator_check_error;
            }
        }
        if (oplen == 2) {
            // Both operands need to have same vec_size(order)
            // For || and &&
            if (oprands[0]->vec_size != oprands[1]->vec_size) {
                fprintf(errorFile, "%d: %s operator must have same vec order\n",
                        ast->line, get_binary_op_str(op));
                goto ast_operator_check_error;
            }
        }
    } else if (is_in_set(arithmetic_ops, 6, op) ||
               is_in_set(comparison_ops, 6, op)) {
        // All operands must have arithmetic types
        for (size_t i = 0; i < oplen; i++) {
            if (!is_in_set(arithmetic_types, 6, oprands[i]->type_code)) {
                fprintf(errorFile,
                        "%d: %s operator must have arithmetic type as %ld "
                        "operand\n",
                        ast->line, get_binary_op_str(op), i + 1);
                goto ast_operator_check_error;
            }
        }
        switch (op) {
            case '*':
                // ss sv vv vs
                assert(oplen == 2);
                if ((oprands[0]->vec_size > 1 && oprands[1]->vec_size > 1) &&
                    (oprands[0]->vec_size != oprands[1]->vec_size)) {
                    fprintf(errorFile,
                            "%d: * operator must have vector of same order as "
                            "operands\n",
                            ast->line);
                    goto ast_operator_check_error;
                }
                break;
            case '/':
            case '^':
            case GEQ:
            case LEQ:
            case '<':
            case '>':
                // ss
                assert(oplen == 2);
                if (!(oprands[0]->vec_size == 1 && oprands[1]->vec_size == 1)) {
                    fprintf(errorFile,
                            "%d: %s operator must have both operands scala "
                            "arithmetic value\n",
                            ast->line, get_binary_op_str(op));
                    goto ast_operator_check_error;
                }
                break;
            case '+':
            case '-':
            case EQ:
            case NEQ:
                // ss vv
                assert(oplen == 2);
                if (!(oprands[0]->vec_size == oprands[1]->vec_size)) {
                    fprintf(
                        errorFile,
                        "%d: %s operator must have vec size of same order\n",
                        ast->line, get_binary_op_str(op));
                    goto ast_operator_check_error;
                }
                break;
            default:
                // Do nothing
                break;
        }
    } else {
        // Operator not in recognized
        fprintf(errorFile,
                "%d: [SEVERE](This is an internal issue and should never "
                "happen) operator not recognized\n",
                ast->line);
        goto ast_operator_check_error;
    }
    return;

ast_operator_check_error:
    ast->type_code = -1;
    ast->vec_size = 0;
}

void ast_condition_check(node* ast) {
    if (ast->kind != IF_STATEMENT_NODE) return;
    int type_cond = ast->if_statement.condition->type_code;
    if (!is_in_set(scala_boolean_types, 3, type_cond)) {
        fprintf(errorFile,
                "%d, condition of if statement must have boolean value as "
                "condition\n",
                ast->line);
    }
}

void ast_function_check(node* ast) {
    // TODO
}

void ast_constructor_check(node* ast) {
    if (ast->kind != CONSTRUCTOR_NODE) return;
}

void ast_assignment_check(node* ast) {
    if (ast->kind != ASSIGNMENT_NODE) return;

    node* dest = ast->binary_node.left;
    node* src = ast->binary_node.right;

    const char* var_name = dest->variable.var_name;
    // Destination need to be a variable node
    assert(var_name != NULL);

    st_entry* ste = scope_find_entry(var_name);
    if (ste == NULL) {
        fprintf(errorFile, "%d: Variable %s used before declaration\n",
                ast->line, var_name);
        goto ast_assignment_check_error;
    }
    if (ste->is_const) {
        fprintf(errorFile, "%d: Assigning value to const variable %s\n",
                ast->line, var_name);
        goto ast_assignment_check_error;
    }
    if (dest->type_code != src->type_code) {
        fprintf(errorFile, "%d: Assigning variable to imcompatible type\n",
                ast->line);
        goto ast_assignment_check_error;
    }
    return;
ast_assignment_check_error:
    ast->type_code = -1;
    ast->vec_size = 0;
}

void ast_declaration_check(node* ast) {
    if (ast->declaration.expr) {
        if (scope_define_symbol(ast->declaration.var_name,
                                ast->is_const, ast->type_code,
                                ast->vec_size)) {
            errorOccurred = 1;
            fprintf(errorFile, "LINE: %d: VariableCan not be declared twice\n",
                    ast->line);
            if (ast->is_const){
                
            }
            if (ast->declaration.expr->type_code != ast->type_code 
                    || ast->declaration.expr->vec_size != ast->vec_size){
                errorOccurred = 1;
                fprintf(errorFile, "LINE: %d: expression should have the same"
                        "type as the declarated variable\n",
                    ast->line);
            }
        }
    } else {
        if (scope_declare_symbol(ast->declaration.var_name,
                                 ast->is_const, ast->type_code,
                                 ast->vec_size)) {
            errorOccurred = 1;
            fprintf(errorFile, "LINE: %d: Variable Can not be declared twice\n",
                    ast->line);
        }
    }
}

void ast_pre_check(node* ast, int depth) {
    node_kind kind = ast->kind;
    if (kind == SCOPE_NODE) {
        scope_enter();
        if (scope_depth() == 0) {  // Root scope predefine vars
            // add pre-defined vars to root scope
            scope_define_symbol("gl_FragColor", 0, VEC_T, 4);
            scope_define_symbol("gl_FragDepth", 0, BOOL_T, 1);
            scope_define_symbol("gl_FragCoord", 0, VEC_T, 4);

            scope_define_symbol("gl_TexCoord", 0, VEC_T, 4);
            scope_define_symbol("gl_Color", 0, VEC_T, 4);
            scope_define_symbol("gl_Secondary", 0, VEC_T, 4);
            scope_define_symbol("gl_gl_FogFragCoord", 0, VEC_T, 4);

            scope_define_symbol("gl_Light_Half", 1, VEC_T, 4);
            scope_define_symbol("gl_Light_Ambient", 1, VEC_T, 4);
            scope_define_symbol("gl_Material_Shininess", 1, VEC_T, 4);

            scope_define_symbol("env1", 1, VEC_T, 4);
            scope_define_symbol("env2", 1, VEC_T, 4);
            scope_define_symbol("env3", 1, VEC_T, 4);
        }
    }
}

void ast_post_check(node* ast, int depth) {
    ast_operator_check(ast);
    ast_condition_check(ast);
    node_kind kind = ast->kind;
    // dispatch to each semantic check functions
    switch (kind) {
        case SCOPE_NODE:
            scope_leave();
            break;
        case EXPRESSION_NODE:
            break;
        case UNARY_EXPRESION_NODE:
            break;
        case BINARY_EXPRESSION_NODE:
            break;
        case INT_NODE:
            break;
        case FLOAT_NODE:
            break;
        case IDENT_NODE:
            break;
        case VAR_NODE:
            break;
        case FUNCTION_NODE:
            break;
        case CONSTRUCTOR_NODE:
            ast->type_code = ast->binary_node.left->type_code;
            ast->vec_size = ast->binary_node.left->vec_size;
            break;
        case TYPE_NODE:
            break;
        case BOOL_NODE:
            break;
        case ARGUMENTS_NODE:
            break;
        case NESTED_EXPRESSION_NODE:
            ast->type_code = ast->unary_node.right->type_code;
            ast->vec_size = ast->unary_node.right->vec_size;
            break;
        case VAR_EXPRESSION_NODE:
            ast->type_code = ast->unary_node.right->type_code;
            ast->vec_size = ast->unary_node.right->vec_size;
            break;
        case STATEMENT_NODE:
            break;
        case IF_STATEMENT_NODE:
            break;
        case WHILE_STATEMENT_NODE:
            break;
        case ASSIGNMENT_NODE:
            break;
        case NESTED_SCOPE_NODE:
            break;
        case DECLARATION_NODE:
            ast->type_code = ast->declaration.type_node->type_code;
            ast->vec_size = ast->declaration.type_node->vec_size;
            ast_declaration_check(ast);
            break;
        case DECLARATIONS_NODE:
            break;
        case UNKNOWN:
            break;
        default:
            break;
    }
}
