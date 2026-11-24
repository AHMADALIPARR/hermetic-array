/*
 * HERMETIC-Ω (hermetic-array) · src/ir_ext.c
 * SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
 * License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
 * Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
 * Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
 * Hark, though this node be but a spark,
 * Its covenant endureth through the dark.
 */

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

/* HERMETIC-Ω IR completion (additions only).
   IR_LOAD carries a materialized array in ->result; IR_RESHAPE and
   IR_PERMUTE take their shape / permutation from ->shape[0..rank);
   IR_EXP (x, v), IR_DOT (a, b), IR_NORM (a), IR_GEO_PROD (a, b),
   IR_EVOLVE (x, v, eta, steps) take operand subtrees in ->args.
   IR_STORE evaluates args[0] and yields a copy. ir_eval_ext always returns
   a freshly owned array in *out, or a negative code with *out == NULL. */

#include "ir_ext.h"
#include "array_ext.h"
#include "geom.h"
#include "geom_ext.h"
#include <stdlib.h>
#include <string.h>

void ir_free(IRNode *ir) {
    if (!ir) return;
    for (size_t i = 0; i < ir->nargs && i < 4; i++) ir_free(ir->args[i]);
    if (ir->result) hao_array_free(ir->result);
    free(ir);
}

IRNode *ir_node_new(IROp op, size_t nargs, IRNode **args) {
    if (nargs > 4) return NULL;
    IRNode *ir = calloc(1, sizeof(IRNode));
    if (!ir) return NULL;
    ir->op = op;
    ir->nargs = nargs;
    for (size_t i = 0; i < nargs; i++) ir->args[i] = args[i];
    return ir;
}

IRNode *ir_load_array(const HaoArray *a) {
    IRNode *ir = ir_node_new(IR_LOAD, 0, NULL);
    if (!ir) return NULL;
    if (hao_array_copy(a, &ir->result) != 0) { free(ir); return NULL; }
    ir->rank = a->rank < 8 ? a->rank : 8;
    for (size_t i = 0; i < ir->rank; i++) ir->shape[i] = a->shape[i];
    return ir;
}

/* Flatten numeric literals; nested array literals of equal length form a matrix. */
static int literal_shape(const Node *pn, size_t *rank, size_t *shape) {
    if (pn->kind == NODE_ATOM) {
        if (pn->glyph) return -3;
        *rank = 0;
        return 0;
    }
    if (pn->kind != NODE_ARRAY) return -3;
    size_t sub_rank = 0, sub_shape[8] = {0};
    for (size_t i = 0; i < pn->nchildren; i++) {
        size_t r, s[8];
        if (literal_shape(pn->children[i], &r, s)) return -3;
        if (i == 0) { sub_rank = r; memcpy(sub_shape, s, sizeof(s)); }
        else {
            if (r != sub_rank) return -2;
            for (size_t k = 0; k < r; k++) if (s[k] != sub_shape[k]) return -2;
        }
    }
    if (sub_rank + 1 > 8) return -4;
    shape[0] = pn->nchildren;
    for (size_t k = 0; k < sub_rank; k++) shape[k + 1] = sub_shape[k];
    *rank = sub_rank + 1;
    return 0;
}

static void literal_fill(const Node *pn, double *dst, size_t *pos) {
    if (pn->kind == NODE_ATOM) { dst[(*pos)++] = pn->literal; return; }
    for (size_t i = 0; i < pn->nchildren; i++) literal_fill(pn->children[i], dst, pos);
}

IRNode *ir_from_node_ext(const Node *pn) {
    if (!pn) return NULL;
    size_t rank, shape[8];
    if (literal_shape(pn, &rank, shape) != 0) return NULL;
    HaoArray *a = hao_array_create(HAO_SCALAR_F64, rank, shape);
    if (!a) return NULL;
    size_t pos = 0;
    if (a->element_count > 0) literal_fill(pn, a->data, &pos);
    IRNode *ir = ir_node_new(IR_LOAD, 0, NULL);
    if (!ir) { hao_array_free(a); return NULL; }
    ir->result = a;
    ir->rank = rank;
    memcpy(ir->shape, shape, rank * sizeof(size_t));
    return ir;
}

static int eval_args(const IRNode *ir, size_t need, HaoArray **vals) {
    if (ir->nargs < need) return -1;
    for (size_t i = 0; i < need; i++) {
        vals[i] = NULL;
        int rc = ir_eval_ext(ir->args[i], &vals[i]);
        if (rc) {
            for (size_t k = 0; k < i; k++) hao_array_free(vals[k]);
            return rc;
        }
    }
    return 0;
}

static void free_args(HaoArray **vals, size_t n) {
    for (size_t i = 0; i < n; i++) hao_array_free(vals[i]);
}

int ir_eval_ext(const IRNode *ir, HaoArray **out) {
    if (!ir || !out) return -1;
    *out = NULL;
    HaoArray *v[4] = {NULL, NULL, NULL, NULL};
    int rc;
    switch (ir->op) {
        case IR_LOAD:
            if (!ir->result) return -1;
            return hao_array_copy(ir->result, out);
        case IR_STORE:
            if ((rc = eval_args(ir, 1, v))) return rc;
            *out = v[0];
            return 0;
        case IR_RESHAPE:
            if ((rc = eval_args(ir, 1, v))) return rc;
            *out = hao_reshape(v[0], ir->rank, ir->shape);
            free_args(v, 1);
            return *out ? 0 : -2;
        case IR_PERMUTE:
            if ((rc = eval_args(ir, 1, v))) return rc;
            if (ir->rank != v[0]->rank) { free_args(v, 1); return -4; }
            rc = hao_transpose_axes(v[0], ir->shape, out);
            free_args(v, 1);
            return rc;
        case IR_EXP:
            if ((rc = eval_args(ir, 2, v))) return rc;
            *out = hao_array_create(HAO_SCALAR_F64, v[0]->rank, v[0]->shape);
            rc = *out ? hao_array_sphere_exp(v[0], v[1], *out) : -1;
            free_args(v, 2);
            if (rc) { hao_array_free(*out); *out = NULL; }
            return rc;
        case IR_DOT: {
            if ((rc = eval_args(ir, 2, v))) return rc;
            double d = hao_inner(v[0], v[1]);
            free_args(v, 2);
            if (d != d) return -2;
            *out = hao_array_create(HAO_SCALAR_F64, 0, NULL);
            if (!*out) return -1;
            ((double *)(*out)->data)[0] = d;
            return 0;
        }
        case IR_NORM:
            if ((rc = eval_args(ir, 1, v))) return rc;
            *out = hao_norm(v[0]);
            free_args(v, 1);
            return *out ? 0 : -1;
        case IR_GEO_PROD:
            if ((rc = eval_args(ir, 2, v))) return rc;
            if (v[0]->element_count == 3 && v[1]->element_count == 3) rc = hao_geometric_product3(v[0], v[1], out);
            else rc = hao_geometric_product(v[0], v[1], out);
            free_args(v, 2);
            return rc;
        case IR_EVOLVE: {
            if ((rc = eval_args(ir, 4, v))) return rc;
            if (v[2]->element_count != 1 || v[3]->element_count != 1) { free_args(v, 4); return -2; }
            double eta = hao_get_f64(v[2], 0);
            double steps = hao_get_f64(v[3], 0);
            if (steps < 0) { free_args(v, 4); return -3; }
            rc = hao_evolve_sphere(v[0], v[1], eta, (size_t)steps, out);
            free_args(v, 4);
            if (rc) *out = NULL;
            return rc;
        }
        default:
            return -1;
    }
}
