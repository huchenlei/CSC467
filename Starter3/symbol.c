#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"

void scope_enter() {

}

void scope_leave();
size_t scope_depth();
int scope_declare_symbol(st_entry st);
st_entry* scope_find_entry(const char* id);
