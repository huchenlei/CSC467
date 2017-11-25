#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "arb.h"
#include "common.h"

#define MAX_INS_LEN 256

void print_insts(inst* instruction) {
    fprintf(outputFile, "!!ARBfp1.0\n");
    inst *cur_ins = instruction;
    while(cur_ins != NULL) {
        if (cur_ins->is_declaration) {

        } else {

        }
        cur_ins = cur_ins->_next;
    }
    fprintf(outputFile, "END\n");
}
