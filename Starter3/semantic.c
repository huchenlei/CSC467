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
static const int literal_node[4] = {TRUE_C, FALSE_C, INT_C, FLOAT_C};

int is_in_set(const int* arr, size_t len, int target) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target) {
            return 1;  // true
        }
    }
    return 0;  // false
}

int vec_to_scala(int type_code) {
    assert(is_in_set(vec_boolean_types, 1, type_code) ||
           is_in_set(vec_arithmetic_types, 2, type_code));
    switch (type_code) {
        case BVEC_T:
            return BOOL_T;
        case IVEC_T:
            return INT_T;
        case VEC_T:
            return FLOAT_T;
    }
    return -1;
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

    if (oplen == 1) {
        ast->is_const = oprands[0]->is_const;
    } else {
        ast->is_const = oprands[0]->is_const && oprands[1]->is_const;
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
            case UMINUS:
                assert(oplen = 1);
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
    errorOccurred = 1;
    ast->type_code = -1;
    ast->vec_size = 0;
}

void ast_condition_check(node* ast) {
    if (ast->kind != IF_STATEMENT_NODE) return;
    int type_cond = ast->if_statement.condition->type_code;
    if (!is_in_set(scala_boolean_types, 3, type_cond)) {
        fprintf(errorFile,
                "%d: condition of if statement must have boolean value as "
                "condition\n",
                ast->line);
    }
}

void ast_function_check(node* ast) {
    if (ast->kind != FUNCTION_NODE) return;
    node* argument_node = ast->func_expr.args;
    ast->is_const = 0;
    switch (ast->func_expr.func_name) {
        case 0:  // dp3
            if (argument_node->argument.arg_size != 2) {
                errorOccurred = 1;
                fprintf(errorFile, "LINE: %d, dp3 take exactly two arguments!",
                        ast->line);
            }
            if ((argument_node->type_code != IVEC_T &&
                 argument_node->type_code != VEC_T) ||
                (argument_node->vec_size != 3 &&
                 argument_node->vec_size != 4)) {
                errorOccurred = 1;
                fprintf(errorFile,
                        "LINE: %d, dp3 only takes vec3(4) ivec3(4)"
                        "as arguments %d, %d",
                        ast->line, argument_node->type_code,
                        argument_node->vec_size);
            }
            if (argument_node->type_code == VEC_T) {
                ast->type_code = FLOAT_T;
                ast->vec_size = 1;
            }
            if (argument_node->type_code == IVEC_T) {
                ast->type_code = INT_T;
                ast->vec_size = 1;
            }

            break;
        case 1:  // lit
            if (argument_node->argument.arg_size != 1) {
                errorOccurred = 1;
                fprintf(errorFile, "LINE: %d, lit only one argument!",
                        ast->line);
            }
            if (argument_node->type_code != VEC_T ||
                argument_node->vec_size != 4) {
                errorOccurred = 1;
                fprintf(errorFile, "LINE: %d, lit only take vec4 as argument!",
                        ast->line);
            }
            ast->type_code = VEC_T;
            ast->vec_size = 4;
            break;
        case 2:  // rsq
            if (argument_node->argument.arg_size != 1) {
                errorOccurred = 1;
                fprintf(errorFile, "LINE: %d, rsq only one argument!",
                        ast->line);
            }
            if (argument_node->type_code != FLOAT_T &&
                argument_node->type_code != INT_T) {
                errorOccurred = 1;
                fprintf(errorFile,
                        "LINE: %d, rsq only take float or "
                        "int as argument!",
                        ast->line);
            }
            if (argument_node->type_code == FLOAT_T) {
                ast->type_code = FLOAT_T;
                ast->vec_size = 1;
            }
            if (argument_node->type_code == INT_T) {
                ast->type_code = INT_T;
                ast->vec_size = 1;
            }
            break;
        default:
            break;
    }
}

int constructor_type_check(int cnstr_type, int arg_type) {
    switch (cnstr_type) {
        case VEC_T:
            if (arg_type != FLOAT_T) return 1;
            break;
        case IVEC_T:
            if (arg_type != INT_T) return 1;
            break;
        case BVEC_T:
            if (arg_type != BOOL_T) return 1;
            break;
        default:
            if (cnstr_type != arg_type) {
                return 1;
            }
            break;
    }
    return 0;
}

void ast_constructor_check(node* ast) {
    if (ast->kind != CONSTRUCTOR_NODE) return;
    node* type_node = ast->binary_node.left;
    node* argument_node = ast->binary_node.right;
    ast->type_code = type_node->type_code;
    ast->vec_size = type_node->vec_size;
    ast->is_const = argument_node->is_const;
    if (constructor_type_check(ast->type_code, argument_node->type_code)) {
        errorOccurred = 1;
        fprintf(errorFile,
                "LINE: %d, arguments type in construction call "
                "is not consistent\n",
                ast->line);
    }
    if (argument_node->vec_size != 1) {
        errorOccurred = 1;
        fprintf(errorFile,
                "LINE: %d, arguments in construction call should "
                "have demansion 1",
                ast->line);
    }
    if (ast->vec_size != argument_node->argument.arg_size) {
        errorOccurred = 1;

        fprintf(errorFile,
                "LINE: %d, number of arguments in construction call"
                "is not consistent",
                ast->line);
    }
}

void ast_simple_expr_eval(node* ast) {
    if (ast->kind == NESTED_EXPRESSION_NODE ||
        ast->kind == VAR_EXPRESSION_NODE) {
        node *child = ast->unary_node.right;
        ast->type_code = child->type_code;
        ast->vec_size = child->vec_size;
        ast->is_const = child->is_const;
    }
}

void ast_argument_check(node* ast) {
    if (ast->kind != ARGUMENTS_NODE) return;
    node* args_node = ast->argument.arguments;
    node* expr_node = ast->argument.expr;
    if (!expr_node) {  // arguments_opt -> arguments
        assert(args_node);
        ast->type_code = args_node->type_code;
        ast->vec_size = args_node->vec_size;
        ast->argument.arg_size = args_node->argument.arg_size;
        ast->is_const = args_node->is_const;
    } else if (!args_node) {  // arguments -> expression
        assert(expr_node);
        ast->type_code = expr_node->type_code;
        ast->vec_size = expr_node->vec_size;
        ast->argument.arg_size = 1;
        ast->is_const = expr_node->is_const;
    } else {  // arguments -> arguments , expression
        if (args_node->type_code != expr_node->type_code ||
            args_node->vec_size != expr_node->vec_size) {
            errorOccurred = 1;
            fprintf(errorFile, "LINE: %d, arguments should have same type",
                    ast->line);
        }
        ast->type_code = args_node->type_code;
        ast->vec_size = args_node->vec_size;
        ast->argument.arg_size = args_node->argument.arg_size + 1;
        ast->is_const = expr_node->is_const && args_node->is_const;
    }
}

void ast_assignment_check(node* ast) {
    if (ast->kind != ASSIGNMENT_NODE) return;

    node* dest = ast->binary_node.left;
    node* src = ast->binary_node.right;

    // Default case assignment will not return a typed node
    ast->type_code = -1;
    ast->vec_size = 0;

    const char* var_name = dest->variable.var_name;
    // Destination need to be a variable node
    assert(var_name != NULL);

    st_entry* ste = scope_find_entry(var_name);
    if (ste == NULL) {
        // fprintf(errorFile, "%d: Variable %s used before declaration\n",
        //         ast->line, var_name);
        // Already checked in variable node
        // No need to report error
        goto ast_assignment_check_error;
    }
    if (ste->is_const) {
        fprintf(errorFile, "%d: Assigning value to const variable %s\n",
                ast->line, var_name);
        goto ast_assignment_check_error;
    }
    if (dest->type_code != src->type_code) {
        fprintf(errorFile, "%d: Assigning variable to incompatible type\n",
                ast->line);
        goto ast_assignment_check_error;
    }
    set_inited(ste);
<<<<<<< HEAD
=======
    // Default case assignment will not return a typed node
    ast->type_code = dest->type_code;
    ast->vec_size = dest->vec_size;
>>>>>>> 9278f3c095866e345a8e1c0c583e17205fffe7f3
    return;
ast_assignment_check_error:
    errorOccurred = 1;
}

void ast_declaration_check(node* ast) {
    if (ast->kind != DECLARATION_NODE) return;
    const char* var_name = ast->declaration.var_name;
    assert(var_name != NULL);
    node* type_node = ast->declaration.type_node;
    ast->type_code = type_node->type_code;
    ast->is_const = type_node->is_const;
    ast->vec_size = type_node->vec_size;

    if (ast->declaration.expr) {
        if (scope_define_symbol(var_name, ast->is_const, ast->type_code,
                                ast->vec_size)) {
            fprintf(errorFile, "%d: Variable can not be declared twice\n",
                    ast->line);
            goto ast_declaration_check_error;
        }
        if (ast->declaration.expr->type_code !=
                ast->declaration.type_node->type_code ||
            ast->declaration.expr->vec_size !=
                ast->declaration.type_node->vec_size) {
            fprintf(errorFile,
                    "%d: expression should have the same"
                    " type as the declarated variable(declared as (%d, %d), "
                    "but assigned as (%d, %d))\n",
                    ast->line, ast->declaration.type_node->type_code,
                    ast->declaration.type_node->vec_size,
                    ast->declaration.expr->type_code,
                    ast->declaration.expr->vec_size);
            goto ast_declaration_check_error;
        }
        if (ast->declaration.type_node->is_const) {
            // check whether the expr is an const which can be determined
            // in compile time
            if (!(ast->declaration.expr->is_const)) {
                fprintf(errorFile,
                        "%d: must assign a const value (literal) to const "
                        "variable %s\n",
                        ast->line, var_name);
            }
            goto ast_declaration_check_error;
        }
    } else {
        if (ast->declaration.type_node->is_const) {
            fprintf(errorFile,
                    "%d: const expression must be initialized at declaration\n",
                    ast->line);
            goto ast_declaration_check_error;
        }
        if (scope_declare_symbol(var_name, ast->is_const, ast->type_code,
                                 ast->vec_size)) {
            fprintf(errorFile, "%d: Variable Can not be declared twice\n",
                    ast->line);
            goto ast_declaration_check_error;
        }
    }
    return;
ast_declaration_check_error:
    errorOccurred = 1;
}

void ast_variable_check(node* ast) {
    if (ast->kind != VAR_NODE) return;
    const char* var_name = ast->variable.var_name;
    assert(var_name != NULL);

    st_entry* ste = scope_find_entry(var_name);
    if (ste == NULL) {
        fprintf(errorFile, "%d: Variable '%s' used before declaration\n",
                ast->line, var_name);
        goto ast_variable_check_error;
    }

    if (ast->variable.is_array) {
        // Index check
        if (ste->vec_size == 1) {
            fprintf(errorFile,
                    "%d: Scala variable %s can not be accessed with index\n",
                    ast->line, var_name);
            goto ast_variable_check_error;
        }

        if (ste->vec_size <= ast->variable.index) {
            fprintf(errorFile,
                    "%d: Vector index out of bound. Try to access vector of "
                    "dimension %d with index %d\n",
                    ast->line, ste->vec_size, ast->variable.index);
            goto ast_variable_check_error;
        }
        ast->type_code = vec_to_scala(ste->type_code);
        ast->vec_size = 1;
    } else {
        ast->type_code = ste->type_code;
        ast->vec_size = ste->vec_size;
    }
    ast->is_const = ste->is_const;

    return;
ast_variable_check_error:
    errorOccurred = 1;
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
    ast_assignment_check(ast);
    ast_declaration_check(ast);
    ast_variable_check(ast);
    ast_function_check(ast);
    ast_argument_check(ast);
    ast_constructor_check(ast);
    ast_simple_expr_eval(ast);
}
