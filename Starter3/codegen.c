#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen.h"
#include "common.h"
#include "parser.tab.h"

#define ZERO_VEC "{0.0, 0.0, 0.0, 0.0}"
#define ONE_VEC "{1.0, 1.0, 1.0, 1.0}"

#define BOOL_FALSE ZERO_VEC
#define BOOL_TRUE ONE_VEC

// Global vars
inst* head = NULL;
inst* tail = NULL;
size_t temp_reg_count = 0;

// Helper functions
void if_ex_func(node* ast, int depth);
void to_arb_pre(node* ast, int depth);
void to_arb_post(node* ast, int depth);
void handle_math_expr(node* ast);
void handle_imm_val(node* ast);
void handle_function(node* ast);
void handle_constructor(node* ast);
void handle_var_expr(node* ast);
void handle_declaration(node* ast);
void handle_assignment(node* ast);

// Append a new instruction at the end of linked list
void append_inst(inst_code c, char* out, char* in1, char* in2, char* in3);
// return the temp reg name
// @warning: The memory used need to be freed
char* assign_temp_reg();

// Util functions
static const int INT_TYPES[] = {INT_T, INT_C, IVEC_T,
                                -1};  // -1 marks the end of array

int is_in_set(const int* arr, int target) {
    assert(target != -1);
    size_t i = 0;
    while (arr[i] != -1) {
        if (arr[i] == target) {
            return 1;  // true
        }
        i++;
    }
    return 0;  // false
}

inst* to_arb(node* root) {
    ast_visit(root, 0, &to_arb_pre, &to_arb_post, 1, &if_ex_func, NULL, NULL);
    return head;
}
void if_ex_pre(node* ast, char* pass_str){
    if (!ast->condi_reg_name){
        ast->condi_reg_name = (char*)calloc(MAX_VAR_LEN, sizeof(char));
            
    }
    strncpy(ast->condi_reg_name, pass_str, MAX_VAR_LEN);
}

void if_ex_func(node* ast, int is_else){
    if (ast->kind != IF_STATEMENT_NODE) return;
    char* condi_reg_name = ast->if_statement.condition->reg_name;
    if (!is_else){
        ast->follow_condi_reg_name = assign_temp_reg();
        append_inst(MOV, ast->follow_condi_reg_name, condi_reg_name,"", "");
        if (ast->condi_reg_name){
            //for nested if, cur_condition = prev_condi && if_condi
            append_inst(MUL, ast->follow_condi_reg_name, ast->condi_reg_name, ast->follow_condi_reg_name, "");
        }
        //only mark successors first time
        ast_visit(ast->if_statement.inside_if, 0, NULL, NULL, 0, NULL, &if_ex_pre, ast->follow_condi_reg_name);
        ast_visit(ast->if_statement.inside_else, 0, NULL, NULL, 0, NULL, &if_ex_pre, ast->follow_condi_reg_name);
    }
    else{
        //compute !if_condi
        append_inst(SUB, ast->follow_condi_reg_name, BOOL_TRUE, condi_reg_name, "");
        if (ast->condi_reg_name){
            //for nested if, cur_condition = prev_condi && !if_condi
            append_inst(MUL, ast->follow_condi_reg_name, ast->condi_reg_name, ast->follow_condi_reg_name, "");
        } 
    }
    
}


void to_arb_pre(node* ast, int depth) {
    // Do nothing
}

void to_arb_post(node* ast, int depth) {
    switch (ast->kind) {
        // All nodes that stores temporary expression values
        case UNARY_EXPRESION_NODE:
        case BINARY_EXPRESSION_NODE:
        case INT_NODE:
        case FLOAT_NODE:
        case BOOL_NODE:
        case FUNCTION_NODE:
        case CONSTRUCTOR_NODE:
            ast->reg_name = assign_temp_reg();
            break;
        default:
            // Do nothing
            break;
    }
    handle_math_expr(ast);
    handle_imm_val(ast);
    handle_function(ast);
    handle_constructor(ast);
    handle_var_expr(ast);
    handle_declaration(ast);
    handle_assignment(ast);
}

void handle_math_expr(node* ast) {
    char* out = ast->reg_name;
    switch (ast->kind) {
        case UNARY_EXPRESION_NODE:
            switch (ast->unary_expr.op) {
                case UMINUS:
                    append_inst(SUB, out, ZERO_VEC,
                                ast->unary_expr.right->reg_name, "");
                    break;
                case '!':
                    append_inst(SUB, out, BOOL_TRUE,
                                ast->unary_expr.right->reg_name, "");
                    break;
                default:
                    assert(0);
                    break;
            }
            break;
        case BINARY_EXPRESSION_NODE: {
            char* opr1 = ast->binary_expr.left->reg_name;
            char* opr2 = ast->binary_expr.right->reg_name;
            switch (ast->binary_expr.op) {
                case '*':
                    append_inst(MUL, out, opr1, opr2, "");
                    break;
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
                case '^':
                    append_inst(POW, out, opr1, opr2, "");
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
                case '+':
                    append_inst(ADD, out, opr1, opr2, "");
                    break;
                case '-':
                    append_inst(SUB, out, opr1, opr2, "");
                    break;
                case EQ: {
                    // (opr1 >= opr2) && (opr2 <= opr1)
                    char* temp = assign_temp_reg();
                    append_inst(SUB, out, opr1, opr2, "");
                    append_inst(SUB, temp, opr1, opr2, "");
                    append_inst(CMP, out, out, BOOL_FALSE, BOOL_TRUE);
                    append_inst(CMP, temp, temp, BOOL_FALSE, BOOL_TRUE);
                    append_inst(MUL, out, temp, out, "");
                    break;
                }
                case NEQ: {
                    // (opr1 < opr2) || (opr1 > opr2)
                    char* temp = assign_temp_reg();
                    append_inst(SUB, out, opr1, opr2, "");
                    append_inst(SUB, temp, opr1, opr2, "");
                    append_inst(CMP, out, out, BOOL_TRUE, BOOL_FALSE);
                    append_inst(CMP, temp, temp, BOOL_TRUE, BOOL_FALSE);
                    append_inst(ADD, out, out, temp, "");
                    break;
                }
                case AND:
                    // opr1 * opr2
                    append_inst(MUL, out, opr1, opr2, "");
                case OR:
                    // opr2 + opr1 - (opr1 * opr2)
                    append_inst(MUL, out, opr1, opr2, "");
                    append_inst(ADD, out, opr1, out, "");
                    append_inst(ADD, out, opr2, out, "");
                default:
                    assert(0);
                    break;
            }
            break;
        }
        default:
            return;
    }
}

void handle_imm_val(node* ast) {
    char literal_expr[MAX_INS_LEN];
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
            snprintf(literal_expr, MAX_INS_LEN, "{%.1f, %.1f, %.1f, %.1f}", val,
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
}

void handle_function(node* ast) {
    if (ast->kind != FUNCTION_NODE) return;
    node* argument_node = ast->func_expr.args;
    char* args[2];
    char* out = ast->reg_name;

    int arg_size = argument_node->argument.arg_size;
    int i = 0;
    while (i < arg_size) {
        assert(argument_node != NULL);
        node* cur_expr = argument_node->argument.expr;
        if (cur_expr != NULL) {
            args[i] = cur_expr->reg_name;
            i++;
        }
        argument_node = argument_node->argument.arguments;
    }

    switch (ast->func_expr.func_name) {
        case 0: {  // dp3
            assert(arg_size == 2);
            append_inst(DP3, out, args[0], args[1], "");
            break;
        }
        case 1:  // lit
            assert(arg_size == 1);
            append_inst(LIT, out, args[0], "", "");
            break;
        case 2:  // rsq
            assert(arg_size == 1);
            append_inst(RSQ, out, args[0], "", "");
            break;
        default:
            assert(0);
            break;
    }
}

static const char* REG_INDEX[] = {"x", "y", "z", "w"};
void handle_constructor(node* ast) {
    if (ast->kind != CONSTRUCTOR_NODE) return;
    char* out = ast->reg_name;
    node* argument_node = ast->binary_node.right;

    int arg_size = argument_node->argument.arg_size;
    assert(arg_size <= 4 && arg_size > 0);
    char var[MAX_VAR_LEN];
    int i = 0;
    while (i < arg_size) {
        assert(argument_node != NULL);
        node* cur_expr = argument_node->argument.expr;
        if (cur_expr != NULL) {
            snprintf(var, MAX_VAR_LEN, "%s.%s", out, REG_INDEX[i]);
            append_inst(MOV, var, cur_expr->reg_name, "", "");
            i++;
        }
        argument_node = argument_node->argument.arguments;
    }
}

#define STRING_MAP(PATTERN, TO)           \
    if (strcmp(PATTERN, var_name) == 0) { \
        strcpy(reg_name, TO);             \
        goto var_expr_end_mapping;        \
    }
void handle_var_expr(node* ast) {
    switch (ast->kind) {
        case VAR_NODE: {
            char* var_name = ast->variable.var_name;
            assert(var_name != NULL);
            char* reg_name = (char*)calloc(MAX_VAR_LEN, sizeof(char));
            STRING_MAP("gl_FragColor", "result.color");
            STRING_MAP("gl_FragDepth", "result.depth");
            STRING_MAP("gl_FragCoord", "fragment.position");
            STRING_MAP("gl_TexCoord", "fragment.texcoord");
            STRING_MAP("gl_Color", "fragment.color");
            STRING_MAP("gl_Secondary", "fragment.color.secondary");
            STRING_MAP("gl_gl_FogFragCoord", "fragment.fogcoord");
            STRING_MAP("gl_Light_Half", "state.light[0].half");
            STRING_MAP("gl_Light_Ambient", "state.lightmodel.ambient");
            STRING_MAP("gl_Material_Shininess", "state.material.shininess");
            STRING_MAP("env1", "program.env[1]");
            STRING_MAP("env2", "program.env[2]");
            STRING_MAP("end3", "program.env[3]");
            // Not system variable
            // Use var_[DEPTH] as unique reg name
            snprintf(reg_name, MAX_VAR_LEN, "%s_%d", var_name,
                     ast->scope_id);
        var_expr_end_mapping:
            if (ast->variable.is_array) {
                strcat(reg_name, ".");
                strcat(reg_name, REG_INDEX[ast->variable.index]);
            }
            ast->reg_name = reg_name;
            break;
        }
        case VAR_EXPRESSION_NODE:
            assert(ast->reg_name == NULL);
            ast->reg_name = (char*)calloc(MAX_VAR_LEN, sizeof(char));
            strcpy(ast->reg_name, ast->unary_node.right->reg_name);
            break;
        default:
            return;
    }
}

void handle_declaration(node* ast) {
    if (ast->kind != DECLARATION_NODE) return;
    char reg_name[MAX_VAR_LEN];
    snprintf(reg_name, MAX_VAR_LEN, "%s_%d", ast->declaration.var_name,
             ast->scope_id);
    append_inst(TEMP, reg_name, "", "", "");
    if (ast->declaration.expr != NULL) {
        append_inst(MOV, reg_name, ast->declaration.expr->reg_name, "", "");
    }
}

void handle_assignment(node* ast) {
    if (ast->kind != ASSIGNMENT_NODE) return;
    node* dest = ast->binary_node.left;
    node* src = ast->binary_node.right;
    char* des_var = dest->reg_name;
    char* src_var = src->reg_name;
    if (ast->condi_reg_name){
        char* temp = assign_temp_reg();
        //(0-TRUE)<0
        append_inst(SUB, temp, BOOL_FALSE, ast->condi_reg_name, "");
        append_inst(CMP, des_var, temp, src_var, des_var);
        free(temp);
    }
    else{
        append_inst(MOV, des_var, src_var, "", "");
    }
    
    
}

void print_insts(inst* instruction) {
    fprintf(outputFile, "!!ARBfp1.0\n");
    inst* cur_ins = instruction;
    char ins_str[MAX_INS_LEN];

    while (cur_ins != NULL) {
        if (cur_ins->code == TEMP) {
            snprintf(ins_str, MAX_INS_LEN, "TEMP %s;", cur_ins->out);
        } else {
            snprintf(ins_str, MAX_INS_LEN, "%s %s", INST_STRING[cur_ins->code],
                     cur_ins->out);
            for (size_t i = 0; i < 3; i++) {
                if (strcmp(cur_ins->in[i], "") == 0) break;
                strcat(ins_str, ", ");
                strcat(ins_str, cur_ins->in[i]);
            }
            strcat(ins_str, ";");
        }
        cur_ins = cur_ins->_next;
        fprintf(outputFile, "%s\n", ins_str);
    }
    fprintf(outputFile, "END\n");
}

void append_inst(inst_code c, char* out, char* in1, char* in2, char* in3) {
    inst* ins = (inst*)malloc(sizeof(inst));
    ins->code = c;
    assert(out != NULL);
    assert(in1 != NULL);
    assert(in2 != NULL);
    assert(in3 != NULL);

    strncpy(ins->out, out, MAX_VAR_LEN);
    strncpy(ins->in[0], in1, MAX_VAR_LEN);
    strncpy(ins->in[1], in2, MAX_VAR_LEN);
    strncpy(ins->in[2], in3, MAX_VAR_LEN);

    ins->_next = NULL;

    if (tail == NULL) {
        assert(head == NULL);
        head = ins;
    } else {
        tail->_next = ins;
    }
    tail = ins;
}

void clear_all_inst(inst* head) {
    inst* cur = head;
    while (cur != NULL) {
        inst* to_des = cur->_next;
        free(cur);
        cur = to_des;
    }
}

char* assign_temp_reg() {
    assert(temp_reg_count + 1 < SIZE_MAX);
    char* temp_reg = (char*)calloc(MAX_VAR_LEN, sizeof(char));
    snprintf(temp_reg, MAX_VAR_LEN, "tempVar%lu", temp_reg_count++);
    append_inst(TEMP, temp_reg, "", "", "");
    return temp_reg;
}
