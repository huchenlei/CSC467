
#ifndef AST_H_
#define AST_H_ 1

#include <stdarg.h>

// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
typedef struct node_ node;
extern node *ast;

typedef enum {
  UNKNOWN               = 0,

  SCOPE_NODE            = (1 << 0),

  EXPRESSION_NODE       = (1 << 2),
  UNARY_EXPRESION_NODE  = (1 << 2) | (1 << 3),
  BINARY_EXPRESSION_NODE= (1 << 2) | (1 << 4),
  INT_NODE              = (1 << 2) | (1 << 5), 
  FLOAT_NODE            = (1 << 2) | (1 << 6),
  IDENT_NODE            = (1 << 2) | (1 << 7),
  VAR_NODE              = (1 << 2) | (1 << 8),
  FUNCTION_NODE         = (1 << 2) | (1 << 9),
  CONSTRUCTOR_NODE      = (1 << 2) | (1 << 10),
  TYPE_NODE             = (1 << 2) | (1 << 11),
  BOOL_NODE             = (1 << 2) | (1 << 12),
  ARGUMENTS_NODE        = (1 << 2) | (1 << 13),
  NESTED_EXPRESSION_NODE= (1 << 2) | (1 << 14),
  VAR_EXPRESSION_NODE   = (1 << 2) | (1 << 15),

  STATEMENT_NODE        = (1 << 1),
  IF_STATEMENT_NODE     = (1 << 1) | (1 << 11),
  WHILE_STATEMENT_NODE  = (1 << 1) | (1 << 12),
  ASSIGNMENT_NODE       = (1 << 1) | (1 << 13),
  NESTED_SCOPE_NODE     = (1 << 1) | (1 << 14),

  DECLARATION_NODE      = (1 << 15),
  DECLARATIONS_NODE     = (1 << 16)
} node_kind;

struct node_ {

  // an example of tagging each node with a type
  node_kind kind;
  int line;
  union {
    struct {
      // declarations?
      // statements?
    } scope;
	
	struct {
 	  int type_code;
	  int vec_size;
	} type;
	
	struct {
	  int func_name;
	  node *args;
	} func_expr;

    struct {
      int op;
      node *right;
    } unary_expr;

    struct {
      int op;
      node *left;
      node *right;
    } binary_expr;

    struct {
      node *right;
    } unary_node;

    struct {
      node *left;
      node *right;
    } binary_node;

    struct {
      int type_code;
      union {
        int int_val;
        float float_val;
      };
    } literal_expr;
	
	struct {
	  char *var_name;
	  int is_array;
	  int index;
          int type_code;
          int vec_size;
	} variable;
	
    struct {
      int is_const;
      char *var_name;
      node *type_node;
      node *expr;
    } declaration;

    struct {
      node *condition;
      node *inside_if;
      node *inside_else;

    } if_statement;

    // etc.
  };
};

node *ast_allocate(node_kind type, int yyline, ...);
void ast_free(node *ast);
void ast_print(node * ast);
void ast_pre_print(node *ast, int depth);
void ast_post_print(node *ast, int depth);
void ast_visit(node * ast, int depth, void(*pre_func)(node*,int), void(*post_func)(node*,int));

#endif /* AST_H_ */
