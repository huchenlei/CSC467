#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"
#include "symbol.h"

symbol_table* cur_scope = NULL;

void scope_enter() {
    printf("symbol: Entering scope\n");
    int is_root_scope = (cur_scope == NULL);
    if (is_root_scope) {
        // Initialize root scope
        cur_scope = malloc(sizeof(symbol_table));
        cur_scope->depth = 0;
        cur_scope->parent_scope = NULL;
    } else {
        // Enter a new scope
        symbol_table* new_st = malloc(sizeof(symbol_table));
        new_st->parent_scope = cur_scope;
        new_st->depth = cur_scope->depth + 1;
        cur_scope = new_st;
    }

    cur_scope->max_entry = BASE_ENTRY_NUM;
    cur_scope->entry_num = 0;
    cur_scope->head = malloc(BASE_ST_SIZE);
    cur_scope->tail = cur_scope->head;

    if (is_root_scope) {
        // add pre-defined vars to root scope
        scope_define_symbol("gl_FragColor", 0, VEC_T, 4);
        scope_define_symbol("gl_FragDepth", 0, BOOL_T, 0);
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

void scope_leave() {
    printf("symbol: Leaving scope\n");
    assert(cur_scope != NULL);

    symbol_table st_des = cur_scope;
    cur_scope = cur_scope->parent_scope;

    // Destroy st_des
}
size_t scope_depth() {
    if (cur_scope != NULL)
        return cur_scope->depth;
    else
        return -1;
}

st_entry* scope_new_entry() {
    st_entry* prev = cur_scope->tail;
    if (cur_scope->entry_num >= cur_scope->max_entry) {
        // reach max entry num current mem can support
        cur_scope->max_entry *= 2;
    } else {
    }
    cur_scope->entry_num++;
}

int scope_declare_symbol(const char* name, int is_const, int type_code,
                         int vec_size) {
    assert(cur_scope != NULL);
    assert(name != NULL);
    assert(vec_size >= 0 && vec_size <= 4);

    st_entry* ste = scope_find_entry(name);
    if (!(ste == NULL || ste->_owner != cur_scope)) {
        // can not declare symbol with same name in a scope more than once
        return 1;
    }
}

void set_inited(st_entry* ste) { ste->has_init = 1; }

st_entry* scope_find_entry(const char* name) { return (st_entry*)NULL; }

int scope_define_symbol(const char* name, int is_const, int type_code,
                        int vec_size) {
    int err = scope_declare_symbol(name, is_const, type_code, vec_size);
    if (err) return err;
    st_entry* ste = scope_find_entry(name);
    ste->has_init = 1;
}
