/* HERMETIC ARRAY Ω (HAΩ)
 * Copyright (C) 2026 AHMAD ALI PARR
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
