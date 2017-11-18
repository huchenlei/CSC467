Bouns:
- const qualified variables must be initialized with constant expressions.
- report the line number on which the sematic error occured
- Ensure that every variable has been assigned a value before being read.
- Provide a contextual message that gives extra meaning to the error to help the programmer debug the error:
	- if the error involves writing to aconst qualified variable,then report the line number where that variable was declared
	- if double declara a variable, then report the line number where that variable was declared
	- report type or variable name for some errors

approach to type/semantic checking:
	- pass type_code, vec_size, is_const from bottom.
	ARGUMENTS check:
		- check arguments_size in ARGUMENTS_NODE, and pass it from bottom. (ex, size++ for [each arguments -> arguments , expression] node)
		- pass is_const, type_code, vec_size from bottom
		 

approach to AST structrure:
	- create struct type for each node
	- call ast_allocate() in parser.y to pass values and children and connect nodes
	- also pass line_num and stored in each node for later use
	- each node contains type_code, vec_size and is_const value, for some nodes such as type_node and declaration node, set those values based on passed arguments
breakdown of work:
RONGZHEN CUI (1000221823):
	- construct AST(ast.h, ast.c)
    - sematic check
      - ast_function_check
      - ast_argument_check
      - ast_constructor_check
      - ast_simple_expr_eval	
CHENLEI HU (1002030651):
	- construct symbol table(symbol.h, symbol.c)
    - sematic check
      - ast_operator_check
      - ast_condition_check
      - ast_assignment_check
      - ast_declaration_check
      - ast_variable_check


challenges faced, novelties:
