#include "ir.h"
#include "array.h"
#include "geom.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

IRNode *ir_from_node(void *parse_node) {
    Node *pn = (Node *)parse_node;
    if (!pn) return NULL;
    IRNode *ir = calloc(1, sizeof(IRNode));
    if (!ir) return NULL;
    switch (pn->kind) {
        case NODE_ATOM:
            if (pn->glyph && strcmp(pn->glyph, "exp") == 0) ir->op = IR_EXP;
            else ir->op = IR_LOAD;
            break;
        case NODE_ARRAY:
            ir->op = IR_LOAD;
            break;
        default:
            ir->op = IR_LOAD;
    }
    return ir;
}

int ir_eval(IRNode *ir, HaoArray **out) {
    if (!ir) return -1;
    switch (ir->op) {
        case IR_EXP: {
            if (ir->nargs < 2) return -1;
            HaoArray *x = NULL, *v = NULL;
            if (ir_eval(ir->args[0], &x) || ir_eval(ir->args[1], &v)) return -1;
            return hao_array_sphere_exp(x, v, *out);
        }
        case IR_DOT:
        case IR_NORM:
        case IR_RESHAPE:
        case IR_PERMUTE:
            /* dispatch to array ops */
            return 0;
        default:
            return -1;
    }
}
