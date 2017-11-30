#ifndef ARB_H
#define ARB_H

#include "ast.h"

/**
    Convert ast node to corresponding arb assembly
*/
// `TEMP` is NOT part of instruction set
// Only used to declare temporary var
#define FOREACH_INST(INST) \
    INST(TEMP)             \
    INST(ABS)              \
    INST(ADD)              \
    INST(CMP)              \
    INST(COS)              \
    INST(DP3)              \
    INST(DP4)              \
    INST(DPH)              \
    INST(DST)              \
    INST(EX2)              \
    INST(FLR)              \
    INST(FRC)              \
    INST(KIL)              \
    INST(LG2)              \
    INST(LIT)              \
    INST(LRP)              \
    INST(MAD)              \
    INST(MAX)              \
    INST(MIN)              \
    INST(MOV)              \
    INST(MUL)              \
    INST(POW)              \
    INST(RCP)              \
    INST(RSQ)              \
    INST(SCS)              \
    INST(SGE)              \
    INST(SIN)              \
    INST(SLT)              \
    INST(SUB)              \
    INST(SWZ)              \
    INST(TEX)              \
    INST(TXB)              \
    INST(TXP)              \
    INST(XPD)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define MAX_INS_LEN 256
#define MAX_VAR_LEN 64

typedef enum { FOREACH_INST(GENERATE_ENUM) } inst_code;
static const char* INST_STRING[] = {FOREACH_INST(GENERATE_STRING)};

typedef struct _inst {
    inst_code code;
    char out[MAX_VAR_LEN];
    char in[3][MAX_VAR_LEN]; // ARB has at most 3 inputs

    // Linked list structure to loop over all insts
    struct _inst* _next;
} inst;

// Convert ast to arb instructions
// @returns instruction linked list
inst* to_arb(node* root);
// output to outputFile
void print_insts(inst* instruction);
// Free all memory used by insts
// Need to be called from outside
void clear_all_inst(inst* head);

#endif
