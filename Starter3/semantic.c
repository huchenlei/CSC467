#include "semantic.h"
#include "ast.h"
#include "common.h"

void ast_pre_check(node *ast, int depth);
void ast_post_check(node *ast, int depth);


int semantic_check(node *ast) {
    
    ast_visit(ast, 0, &ast_pre_check, &ast_post_check);
    return 0;
}

void ast_pre_check(node *ast, int depth){
    node_kind kind = ast->kind;
    if (kind == SCOPE_NODE){
        scope_enter();
    }
}
void ast_post_check(node *ast, int depth){
    node_kind kind = ast->kind;
  // dispatch to each semantic check functions
    switch(kind) {
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
            break;
        case VAR_EXPRESSION_NODE:
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
            break;
        case DECLARATIONS_NODE:
            break;
        case UNKNOWN:
            break;
        default:
            break;
    }
  
}
