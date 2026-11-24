/*
 * HERMETIC-Ω (hermetic-array) · tests/test_ir_ext.c
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

#include "../src/array.h"
#include "../src/array_ext.h"
#include "../src/parser.h"
#include "../src/ir.h"
#include "../src/ir_ext.h"
#include <stdio.h>
#include <math.h>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static IRNode *lit(const char *src) {
    Node *t = parse(src);
    IRNode *ir = ir_from_node_ext(t);
    node_free(t);
    return ir;
}

static IRNode *scalar(double d) {
    HaoArray *a = hao_array_create(HAO_SCALAR_F64, 0, NULL);
    ((double *)a->data)[0] = d;
    IRNode *ir = ir_load_array(a);
    hao_array_free(a);
    return ir;
}

int main(void) {
    hao_pool_init(1 << 20);
    HaoArray *out = NULL;

    /* array literals from the original parser lower to IR_LOAD */
    IRNode *x = lit("[1 0 0]");
    CHECK(x && x->op == IR_LOAD && x->rank == 1 && x->shape[0] == 3);
    CHECK(ir_eval_ext(x, &out) == 0 && out->element_count == 3 && hao_get_f64(out, 0) == 1);
    hao_array_free(out);
    IRNode *m = lit("[[1,2,3],[4,5,6]]");
    CHECK(m && m->rank == 2 && m->shape[0] == 2 && m->shape[1] == 3);
    CHECK(lit("[[1,2],[3]]") == NULL);
    CHECK(lit("exp") == NULL);

    /* NORM, DOT */
    IRNode *a[4];
    a[0] = lit("[3 4]");
    IRNode *norm = ir_node_new(IR_NORM, 1, a);
    CHECK(ir_eval_ext(norm, &out) == 0 && fabs(hao_get_f64(out, 0) - 5.0) < 1e-12);
    hao_array_free(out);
    a[0] = lit("[1 2 3]"); a[1] = lit("[4 5 6]");
    IRNode *dot = ir_node_new(IR_DOT, 2, a);
    CHECK(ir_eval_ext(dot, &out) == 0 && out->rank == 0 && hao_get_f64(out, 0) == 32);
    hao_array_free(out);

    /* RESHAPE, PERMUTE */
    a[0] = lit("[1 2 3 4 5 6]");
    IRNode *rs = ir_node_new(IR_RESHAPE, 1, a);
    rs->rank = 2; rs->shape[0] = 2; rs->shape[1] = 3;
    CHECK(ir_eval_ext(rs, &out) == 0 && out->rank == 2 && out->shape[1] == 3);
    hao_array_free(out);
    rs->shape[1] = 4;
    CHECK(ir_eval_ext(rs, &out) == -2 && out == NULL);
    a[0] = m;
    IRNode *pm = ir_node_new(IR_PERMUTE, 1, a);
    pm->rank = 2; pm->shape[0] = 1; pm->shape[1] = 0;
    CHECK(ir_eval_ext(pm, &out) == 0 && out->shape[0] == 3 && hao_get_f64(out, 1) == 4);
    hao_array_free(out);

    /* EXP, EVOLVE: unit-norm preservation through the IR */
    a[0] = lit("[1 0 0]"); a[1] = lit("[0 0.1 0]");
    IRNode *ex = ir_node_new(IR_EXP, 2, a);
    CHECK(ir_eval_ext(ex, &out) == 0 && fabs(hao_inner(out, out) - 1.0) < 1e-12);
    hao_array_free(out);
    a[0] = lit("[2 0 0]"); a[1] = lit("[0 0.1 0]");
    IRNode *bad = ir_node_new(IR_EXP, 2, a);
    CHECK(ir_eval_ext(bad, &out) == -3 && out == NULL);
    a[0] = lit("[1 0 0]"); a[1] = lit("[0 0.05 0]"); a[2] = scalar(1.0); a[3] = scalar(3.0);
    IRNode *ev = ir_node_new(IR_EVOLVE, 4, a);
    CHECK(ir_eval_ext(ev, &out) == 0 && fabs(hao_get_f64(out, 0) - 0.9962554647802354) < 1e-12);
    hao_array_free(out);

    /* GEO_PROD on 3-vectors returns scalar + full bivector */
    a[0] = lit("[1 0 0]"); a[1] = lit("[0 1 0]");
    IRNode *gp = ir_node_new(IR_GEO_PROD, 2, a);
    CHECK(ir_eval_ext(gp, &out) == 0 && out->element_count == 4 && hao_get_f64(out, 3) == 1);
    hao_array_free(out);

    /* STORE passes its operand through */
    a[0] = lit("[7 8]");
    IRNode *st = ir_node_new(IR_STORE, 1, a);
    CHECK(ir_eval_ext(st, &out) == 0 && hao_get_f64(out, 1) == 8);
    hao_array_free(out);

    ir_free(x); ir_free(norm); ir_free(dot); ir_free(rs); ir_free(pm);
    ir_free(ex); ir_free(bad); ir_free(ev); ir_free(gp); ir_free(st);
    hao_pool_free();
    if (failures) { printf("ir ext tests: %d failure(s)\n", failures); return 1; }
    printf("ir ext tests passed\n");
    return 0;
}
