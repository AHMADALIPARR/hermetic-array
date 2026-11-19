#ifndef HAO_IR_H
#define HAO_IR_H

#include "array.h"
#include "parser.h"

typedef enum {
    IR_LOAD,
    IR_STORE,
    IR_RESHAPE,
    IR_PERMUTE,
    IR_EXP,
    IR_DOT,
    IR_NORM,
    IR_GEO_PROD,
    IR_EVOLVE
} IROp;

typedef struct IRNode {
    IROp op;
    HaoArray *result;
    struct IRNode *args[4];
    size_t nargs;
    size_t shape[8];
    size_t rank;
} IRNode;

IRNode *ir_from_node(void *parse_node);
int ir_eval(IRNode *ir, HaoArray **out);
void ir_free(IRNode *ir);

#endif
