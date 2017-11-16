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
static const int arithmetic_types[6] = {VEC_T, FLOAT_T, FLOAT_C,
    IVEC_T, INT_T, INT_C};

int is_in_set(const int* arr, size_t len, int target) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target) {
            return 1; // true
        }
    }
    return 0; // false
}

void ast_operator_check(node* ast) {
    node_kind kind = ast->kind;
    int op = 0;
    node * oprands[2];
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

    if (is_in_set(logic_ops, 3, op)) {
        // All operands to logical ops must have boolean types
        for (size_t i = 0; i < oplen; i++) {
            if (!is_in_set(boolean_types, 4, oprands[i]->type_code)) {
                fprintf(errorFile,
                        "%d: %s operator must have bool type as %ld operand\n",
                        ast->line, get_binary_op_str(op), i + 1);
            }
        }
    }

    if (is_in_set(arithmetic_ops, 6, op) || is_in_set(comparison_ops, 6, op)) {
        // All operands must have arithmetic types
        for (size_t i = 0; i < oplen; i++) {
            if (!is_in_set(arithmetic_types, 6, oprands[i]->type_code)) {
                fprintf(
                        errorFile,
                        "%d: %s operator must have arithmetic type as %ld operand\n",
                        ast->line, get_binary_op_str(op), i + 1);
            }
        }
    }

    if (oplen == 2) { // binary_expr
        // Both operands need to have same vec_size(order)
        if (oprands[0]->vec_size != oprands[1]->vec_size) {
            fprintf(errorFile, "%d: %s operator must have same vec order\n",
                    ast->line, get_binary_op_str(op));
        }
    }

    // TODO More specific type checks
}

void ast_declaration_check(node *ast) {
    if (ast->declaration.expr) {
        if (scope_define_symbol(ast->declaration.var_name,
                ast->declaration.is_const,
                ast->type_code,
                ast->vec_size)) {
            errorOccurred = 1;
            fprintf(errorFile, "LINE: %d: Variable Can not be declared twice\n", ast->line);
        }
    } else {
        if (scope_declare_symbol(ast->declaration.var_name,
                ast->declaration.is_const,
                ast->type_code,
                ast->vec_size)) {
            errorOccurred = 1;
            fprintf(errorFile, "LINE: %d: Variable Can not be declared twice\n", ast->line);
        }
    }
}

void ast_pre_check(node* ast, int depth) {
    node_kind kind = ast->kind;
    if (kind == SCOPE_NODE) {
        scope_enter();
        if (scope_depth() == 0) { // Root scope predefine vars
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
            //ast_declaration_check(ast);
            break;
        case DECLARATIONS_NODE:
            break;
        case UNKNOWN:
            break;
        default:
            break;
    }
}
