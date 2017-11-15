#include "semantic.h"
#include "ast.h"

int semantic_check(node *ast) {
  node_kind kind = ast->kind;
  // dispatch to each semantic check functions
  switch(kind) {
  case SCOPE_NODE            :
    break;
  case EXPRESSION_NODE       :
    break;
  case UNARY_EXPRESION_NODE  :
    break;
  case BINARY_EXPRESSION_NODE:
    break;
  case INT_NODE              :
    break;
  case FLOAT_NODE            :
    break;
  case IDENT_NODE            :
    break;
  case VAR_NODE              :
    break;
  case FUNCTION_NODE         :
    break;
  case CONSTRUCTOR_NODE      :
    break;
  case TYPE_NODE             :
    break;
  case BOOL_NODE             :
    break;
  case ARGUMENTS_NODE        :
    break;
  case NESTED_EXPRESSION_NODE:
    break;
  case VAR_EXPRESSION_NODE   :
    break;
  case STATEMENT_NODE        :
    break;
  case IF_STATEMENT_NODE     :
    break;
  case WHILE_STATEMENT_NODE  :
    break;
  case ASSIGNMENT_NODE       :
    break;
  case NESTED_SCOPE_NODE     :
    break;
  case  DECLARATION_NODE     :
    break;
  case DECLARATIONS_NODE     :
    break;
  case UNKNOWN:
    break;
  default:
    break;
  }
  return 0; // failed checks
}
