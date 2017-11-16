#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol.h"

symbol_table* cur_scope = NULL;
// Helper functions
st_entry* scope_new_entry();

void scope_enter() {
    printf("symbol: Entering scope\n");
    int is_root_scope = (cur_scope == NULL);
    if (is_root_scope) {
        // Initialize root scope
        cur_scope = (symbol_table*) malloc(sizeof (symbol_table));
        cur_scope->depth = 0;
        cur_scope->parent_scope = NULL;
    } else {
        // Enter a new scope
        symbol_table* new_st = (symbol_table*) malloc(sizeof (symbol_table));
        new_st->parent_scope = cur_scope;
        new_st->depth = cur_scope->depth + 1;
        cur_scope = new_st;
    }

    cur_scope->max_entry = BASE_ENTRY_NUM;
    cur_scope->entry_num = 0;
    cur_scope->head = NULL;
    cur_scope->tail = NULL;
}

void scope_leave() {
    printf("symbol: Leaving scope\n");
    assert(cur_scope != NULL);

    symbol_table* st_des = cur_scope;
    cur_scope = cur_scope->parent_scope;

    // Destroy st_des
    // Destroy all entries
    size_t sanity_count = 0;
    st_entry* cur_node = st_des->head;
    while (cur_node != NULL) {
        if (cur_node->_is_pivot) {
            free(cur_node);
        }
        sanity_count++;
        cur_node = cur_node->_next;
    }
#ifdef DEBUG
    printf("scope_leave: sanity count %ld, entry num %ld\n", sanity_count, st_des->entry_num);
#endif
    assert(sanity_count == st_des->entry_num);
    assert(st_des->max_entry % BASE_ENTRY_NUM == 0);
    assert(st_des->max_entry >= st_des->entry_num);

    free(st_des);
}

size_t scope_depth() {
    if (cur_scope != NULL)
        return cur_scope->depth;
    else
        return -1;
}

st_entry* scope_new_entry() {
    st_entry* prev = cur_scope->tail;
    st_entry* new_st = NULL;
    if (cur_scope->head == NULL) {
        // First call
        new_st = (st_entry*) malloc(BASE_ST_SIZE);
        new_st->_is_pivot = 1;
        cur_scope->head = new_st;
    } else if (cur_scope->entry_num >= cur_scope->max_entry) {
        // reach max entry num current mem can support
        new_st = (st_entry*) malloc(sizeof (st_entry) * cur_scope->max_entry);
        cur_scope->max_entry *= 2;
        new_st->_is_pivot = 1;
    } else {
        // move stack pointer
        new_st = prev + sizeof (st_entry);
        new_st->_is_pivot = 0;
    }
    
    cur_scope->entry_num++;
    cur_scope->tail = new_st;
    if (prev != NULL)
        prev->_next = new_st;
    return new_st;
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

    st_entry* new_st = scope_new_entry();
    new_st->is_const = is_const;
    new_st->type_code = type_code;
    new_st->vec_size = vec_size;
    strncpy(new_st->var_name, name, MAX_NAME_LEN);

    new_st->_next = NULL;
    new_st->_owner = cur_scope;

    return 0;
}

void set_inited(st_entry* ste) {
    ste->has_init = 1;
}

st_entry* scope_find_entry(const char* name) {
    assert(name != NULL);
    assert(cur_scope != NULL);
    symbol_table* scope = cur_scope;
    while (scope != NULL) {
        st_entry* result = scope_find_local_entry(name);
        if (result == NULL) {
            scope = scope->parent_scope;
        } else {
            return result;
        }
    }
    return NULL;
}

st_entry* scope_find_local_entry(const char* name) {
    assert(name != NULL);
    assert(cur_scope != NULL);
    st_entry* cur_entry = cur_scope->head;
    while (cur_entry != NULL) {
        if (strncasecmp(cur_entry->var_name, name, MAX_NAME_LEN) == 0) {
            return cur_entry;
        } else {
            cur_entry = cur_entry->_next;
        }
    }
    return NULL;
}

int scope_define_symbol(const char* name, int is_const, int type_code,
        int vec_size) {
    int err = scope_declare_symbol(name, is_const, type_code, vec_size);
    if (err) return err;
    st_entry* ste = scope_find_entry(name);
    assert(ste != NULL);
    ste->has_init = 1;

    return 0;
}
