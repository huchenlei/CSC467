#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"

void scope_enter() {
    printf("symbol: Entering scope\n");
}

void scope_leave() {
    printf("symbol: Leaving scope\n");
}
size_t scope_depth() {
    return 0;
}

int scope_declare_symbol(const char* name, int is_const, int type_code, int vec_size) {
    return 1;
}

void set_inited() {
    
}

const st_entry* scope_find_entry(const char* id) {
    return (st_entry*)NULL;
}
