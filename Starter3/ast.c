#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {
  
	
  // type
  case TYPE_NODE:
	ast->type.type_name = va_arg(args, int);
	ast->type.vec_num = va_arg(args, int) + 1;
 	break;

  case DECLARATION_NODE:
	ast->declaration.is_const = va_arg(args, int);
	ast->declaration.type_node = va_arg(args, node *);
	ast->declaration.var_name = va_arg(args, char *);
	ast->declaration.expr = va_arg(args, node *);
	break;

  // expression
  case CONSTRUCTOR_NODE:
  case ARGUMENTS_NODE:
  case SCOPE_NODE:
  case DECLARATIONS_NODE:
  case STATEMENT_NODE:
  case ASSIGNMENT_NODE:
	ast->binary_node.left = va_arg(args, node *);
	ast->binary_node.right = va_arg(args, node *); 
	break;
  
  case IF_STATEMENT_NODE:
	ast->if_statement.condition = va_arg(args, node *);
	ast->if_statement.inside_if = va_arg(args, node *);
	ast->if_statement.inside_else = va_arg(args, node *);
	break;

  case FUNCTION_NODE:
	ast->func_expr.func_name = va_arg(args, int);
	ast->func_expr.args = va_arg(args, node *); 
	break;

  case UNARY_EXPRESION_NODE:
	ast->unary_expr.op = va_arg(args, int);
	ast->unary_expr.right = va_arg(args, node *);
	break;

  case BINARY_EXPRESSION_NODE:
    ast->binary_expr.op = va_arg(args, int);
    ast->binary_expr.left = va_arg(args, node *);
    ast->binary_expr.right = va_arg(args, node *);
    break;
  
  case NESTED_EXPRESSION_NODE:
  case VAR_EXPRESSION_NODE:
  case NESTED_SCOPE_NODE:
	ast->unary_node.right = va_arg(args, node *);
	break;

  case BOOL_NODE:
	ast->literal_expr.type_name = BOOL_T;
	ast->literal_expr.int_val = va_arg(args, int);
	break;
	
  case INT_NODE:
	ast->literal_expr.type_name = INT_T;
	ast->literal_expr.int_val = va_arg(args, int);
	break;

  case FLOAT_NODE:
	ast->literal_expr.type_name = FLOAT_T;
	ast->literal_expr.float_val = va_arg(args, double);
  	break;

  case VAR_NODE:
	ast->variable.var_name = va_arg(args, char *);
	ast->variable.is_array = va_arg(args, int);
	ast->variable.length = va_arg(args, int);
	break;


  // ...

  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {
	ast_visit(ast, 0, &ast_pre_print, &ast_post_print);
	fprintf(dumpFile, "\n");
}

const char *get_type_name(node *type_node) {
  switch(type_node->type.type_name) {
    case FLOAT_T:
      return "float";
    case INT_T:
      return "int";
    case BOOL_T:
      return "bool";
    case BVEC_T:
      switch(type_node->type.vec_num){
        case 2:
          return "bvec2";
        case 3:
          return "bvec3";
        case 4:
          return "bvec4";
      }
    case IVEC_T:
      switch(type_node->type.vec_num){
        case 2:
          return "ivec2";
        case 3:
          return "ivec3";
        case 4:
          return "ivec4";
      }
    case VEC_T:
      switch(type_node->type.vec_num){
        case 2:
          return "vec2";
        case 3:
          return "vec3";
        case 4:
          return "vec4";
      }
    default:
      return "???";
  }
}


void print_indent(int depth){
	fprintf(dumpFile, "\n");
	int i;
    for (i = 0; i < depth; i++) {
      fprintf(dumpFile, "    ");
    }
}

void ast_pre_print(node *ast, int depth){
	print_indent(depth);
	fprintf(dumpFile, "(");
	switch(ast->kind){
		case SCOPE_NODE:
			fprintf(dumpFile, "SCOPE ");
			break;

		case DECLARATIONS_NODE:
			fprintf(dumpFile, "DECLARATIONS ");
			break;

		case DECLARATION_NODE:
			fprintf(dumpFile, "DECLARATION %s %s ", ast->declaration.var_name, get_type_name(ast->declaration.type_node));
			break;

		case STATEMENT_NODE:
			fprintf(dumpFile, "STATEMENTS ");
			break;
		
		case ASSIGNMENT_NODE:
			fprintf(dumpFile, "ASSIGN %s %s ", "TYPE??", 
				ast->binary_node.left->variable.var_name); //TODO: need to get the type of variable, implement after symbol table
			break;

		case IF_STATEMENT_NODE:
			fprintf(dumpFile, "IF ");
			break;
		
		default: break;
	}
}



void ast_post_print(node *ast, int depth){
	print_indent(depth);
	fprintf(dumpFile, ")");
}



void ast_visit(node * ast, int depth, void(*pre_func)(node*,int), void(*post_func)(node*,int)){
	if (!ast) return;
	if (pre_func) pre_func(ast, depth);
	
	switch(ast->kind){
		case CONSTRUCTOR_NODE:
  		case ARGUMENTS_NODE:
  		case SCOPE_NODE:
  		case DECLARATIONS_NODE:
  		case STATEMENT_NODE:
			ast_visit(ast->binary_node.left, depth+1, pre_func, post_func);
			ast_visit(ast->binary_node.right, depth+1, pre_func, post_func);
			break;
		case ASSIGNMENT_NODE:
			ast_visit(ast->binary_node.right, depth+1, pre_func, post_func);
			break;

		case DECLARATION_NODE:
			ast_visit(ast->declaration.expr, depth+1, pre_func, post_func);
			break;

		case IF_STATEMENT_NODE:
			ast_visit(ast->if_statement.condition, depth+1, pre_func, post_func);
			ast_visit(ast->if_statement.inside_if, depth+1, pre_func, post_func);
			ast_visit(ast->if_statement.inside_else, depth+1, pre_func, post_func);
			break;

		case FUNCTION_NODE:
			ast_visit(ast->func_expr.args, depth+1, pre_func, post_func);
			break;
		
		case UNARY_EXPRESION_NODE:
			ast_visit(ast->unary_expr.right, depth+1, pre_func, post_func);
			break;

		case BINARY_EXPRESSION_NODE:
			ast_visit(ast->binary_expr.left, depth+1, pre_func, post_func);
			ast_visit(ast->binary_expr.right, depth+1, pre_func, post_func);
			break;
		
  		case NESTED_EXPRESSION_NODE:
  		case VAR_EXPRESSION_NODE:
  		case NESTED_SCOPE_NODE:
			ast_visit(ast->unary_node.right, depth+1, pre_func, post_func);
			break;
		
		default:
			break;
		
	}
	
	if (post_func) post_func(ast, depth);

  
}

