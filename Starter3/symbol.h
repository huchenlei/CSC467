#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <stddef.h>

/**
   Singleton Object Symbol Table
 */

#define BASE_ENTRY_NUM 256
// how many entry space assigned initially for each st
#define BASE_ST_SIZE BASE_ENTRY_NUM * sizeof(struct _st_entry)

struct _st_entry;

struct _st {
  size_t depth;
  size_t entry_num;
  struct _st* parent_scope;
  struct _st_entry* stack_base;
  struct _st_entry* stack;
};

struct _st_entry {

};

typedef _st_entry st_entry;
typedef _st symbol_table;

/**
   Methods of symbol table
   scope_enter: enter a new scope
   scope_leave: destroy all variables associated with current scope
   scope_depth: get the depth of current scope. root scope returns 0
   scope_declare_symbol: declare a symbol in current scope
   scope_find_entry: find the symbol in *previous* scopes; return NULL
                     when symbol is not found
 */

void scope_enter();
void scope_leave();
size_t scope_depth();
int scope_declare_symbol(st_entry st);
st_entry* scope_find_entry(const char* id);

#endif
