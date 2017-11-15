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

int scope_declare_symbol(st_entry st) {
    return 0;
}

st_entry* scope_find_entry(const char* id) {
    return (st_entry*)NULL;
}
