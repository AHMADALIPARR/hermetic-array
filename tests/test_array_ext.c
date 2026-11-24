/*
 * HERMETIC-Ω (hermetic-array) · tests/test_array_ext.c
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
#include <stdio.h>
#include <math.h>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static bool near(double a, double b) { return fabs(a - b) < 1e-12; }

static HaoArray *vec(const double *d, size_t n) {
    HaoArray *a = NULL;
    size_t sh[1] = {n};
    hao_from_f64(d, 1, sh, &a);
    return a;
}

static HaoArray *mat(const double *d, size_t r, size_t c) {
    HaoArray *a = NULL;
    size_t sh[2] = {r, c};
    hao_from_f64(d, 2, sh, &a);
    return a;
}

int main(void) {
    HaoArray *o = NULL, *o2 = NULL;

    /* iota */
    CHECK(hao_iota(5, &o) == 0 && o->element_count == 5 && hao_get_f64(o, 4) == 4.0);
    hao_array_free(o);

    /* reverse along each axis of a 2x3 matrix */
    double m6[6] = {1, 2, 3, 4, 5, 6};
    HaoArray *m = mat(m6, 2, 3);
    CHECK(hao_reverse(m, 0, &o) == 0 && hao_get_f64(o, 0) == 4 && hao_get_f64(o, 5) == 3);
    hao_array_free(o);
    CHECK(hao_reverse(m, 1, &o) == 0 && hao_get_f64(o, 0) == 3 && hao_get_f64(o, 3) == 6);
    hao_array_free(o);
    CHECK(hao_reverse(m, 2, &o) == -4);

    /* transpose: 2x3 -> 3x2 */
    CHECK(hao_transpose(m, &o) == 0 && o->shape[0] == 3 && o->shape[1] == 2);
    CHECK(hao_get_f64(o, 1) == 4 && hao_get_f64(o, 2) == 2 && hao_get_f64(o, 5) == 6);
    hao_array_free(o);

    /* general permutation (3-cycle) on a 2x3x4 array: result axis i = source axis perm[i] */
    double c24[24];
    for (int i = 0; i < 24; i++) c24[i] = i;
    size_t sh3[3] = {2, 3, 4};
    HaoArray *cube = NULL;
    hao_from_f64(c24, 3, sh3, &cube);
    size_t perm[3] = {2, 0, 1};
    CHECK(hao_transpose_axes(cube, perm, &o) == 0);
    CHECK(o->shape[0] == 4 && o->shape[1] == 2 && o->shape[2] == 3);
    /* out[k][i][j] = cube[i][j][k]: out[3][1][2] = cube[1][2][3] = 23 */
    CHECK(hao_get_f64(o, 3 * 6 + 1 * 3 + 2) == 23);
    CHECK(hao_get_f64(o, 1 * 6 + 0 * 3 + 2) == 9);
    hao_array_free(o);
    size_t bad[3] = {0, 0, 1};
    CHECK(hao_transpose_axes(cube, bad, &o) == -4);

    /* reshape with fill: under-fill truncates, over-fill cycles, empty source is a length error */
    double v3[3] = {1, 2, 3};
    HaoArray *a3 = vec(v3, 3);
    size_t s22[2] = {2, 2};
    CHECK(hao_reshape_fill(a3, 2, s22, &o) == 0 && hao_get_f64(o, 3) == 1);
    hao_array_free(o);
    size_t s2[1] = {2};
    CHECK(hao_reshape_fill(a3, 1, s2, &o) == 0 && o->element_count == 2 && hao_get_f64(o, 1) == 2);
    hao_array_free(o);
    HaoArray *empty = NULL;
    size_t s0[1] = {0};
    hao_from_f64(NULL, 1, s0, &empty);
    CHECK(hao_reshape_fill(empty, 1, s2, &o) == -2);
    /* original exact reshape still rejects a mismatch */
    CHECK(hao_reshape(a3, 2, s22) == NULL);

    /* take / drop on major cells */
    CHECK(hao_take(m, 1, &o) == 0 && o->shape[0] == 1 && hao_get_f64(o, 2) == 3);
    hao_array_free(o);
    CHECK(hao_take(m, -1, &o) == 0 && hao_get_f64(o, 0) == 4);
    hao_array_free(o);
    CHECK(hao_take(m, 3, &o) == -2);
    CHECK(hao_drop(a3, 1, &o) == 0 && o->element_count == 2 && hao_get_f64(o, 0) == 2);
    hao_array_free(o);
    CHECK(hao_drop(a3, -2, &o) == 0 && o->element_count == 1 && hao_get_f64(o, 0) == 1);
    hao_array_free(o);

    /* join along the leading axis */
    CHECK(hao_join(a3, a3, &o) == 0 && o->element_count == 6 && hao_get_f64(o, 3) == 1);
    hao_array_free(o);
    CHECK(hao_join(m, a3, &o) == -4);

    /* broadcasting: 3-vector to 2x3, and arithmetic with broadcasting */
    size_t s23[2] = {2, 3};
    CHECK(hao_broadcast(a3, 2, s23, &o) == 0 && hao_get_f64(o, 4) == 2);
    hao_array_free(o);
    CHECK(hao_map2(m, a3, HAO_BIN_ADD, &o) == 0 && hao_get_f64(o, 0) == 2 && hao_get_f64(o, 5) == 9);
    hao_array_free(o);
    double v2[2] = {1, 2};
    HaoArray *a2 = vec(v2, 2);
    CHECK(hao_map2(m, a2, HAO_BIN_MUL, &o) == -2);
    CHECK(hao_map2(a3, a3, HAO_BIN_POW, &o) == 0 && hao_get_f64(o, 2) == 27);
    hao_array_free(o);
    CHECK(hao_scale(a3, 2.0, &o) == 0 && hao_get_f64(o, 2) == 6);
    hao_array_free(o);

    /* reduce / scan */
    CHECK(hao_reduce(m, 0, HAO_BIN_ADD, &o) == 0 && o->rank == 1 && hao_get_f64(o, 0) == 5 && hao_get_f64(o, 2) == 9);
    hao_array_free(o);
    CHECK(hao_reduce(m, 1, HAO_BIN_MAX, &o) == 0 && hao_get_f64(o, 0) == 3 && hao_get_f64(o, 1) == 6);
    hao_array_free(o);
    CHECK(hao_scan(a3, 0, HAO_BIN_ADD, &o) == 0 && hao_get_f64(o, 2) == 6);
    hao_array_free(o);
    CHECK(hao_scan(m, 1, HAO_BIN_MUL, &o) == 0 && hao_get_f64(o, 5) == 120);
    hao_array_free(o);
    CHECK(hao_reduce(empty, 0, HAO_BIN_ADD, &o) == 0 && hao_get_f64(o, 0) == 0);
    hao_array_free(o);
    CHECK(hao_reduce(empty, 0, HAO_BIN_MAX, &o) == -3);

    /* tensor (outer) product, matrix product, contraction */
    CHECK(hao_outer(a2, a3, &o) == 0 && o->rank == 2 && o->shape[0] == 2 && o->shape[1] == 3 && hao_get_f64(o, 5) == 6);
    hao_array_free(o);
    CHECK(hao_matmul(m, a3, &o) == 0 && o->rank == 1 && hao_get_f64(o, 0) == 14 && hao_get_f64(o, 1) == 32);
    hao_array_free(o);
    HaoArray *mt = NULL;
    hao_transpose(m, &mt);
    CHECK(hao_matmul(m, mt, &o) == 0 && o->shape[0] == 2 && o->shape[1] == 2 && hao_get_f64(o, 1) == 32);
    CHECK(hao_contract(o, 0, 1, &o2) == 0 && o2->rank == 0 && near(hao_get_f64(o2, 0), 14 + 77));
    hao_array_free(o2);
    hao_array_free(o);
    CHECK(hao_matmul(m, m, &o) == -2);
    /* contraction of a ⊗ b over its two axes equals the inner product */
    CHECK(hao_outer(a3, a3, &o) == 0 && hao_contract(o, 0, 1, &o2) == 0 && hao_get_f64(o2, 0) == hao_inner(a3, a3));
    hao_array_free(o2);
    hao_array_free(o);

    /* grade (stable) */
    double g5[5] = {3, 1, 2, 1, 5};
    HaoArray *a5 = vec(g5, 5);
    CHECK(hao_grade_up(a5, &o) == 0 && hao_get_f64(o, 0) == 1 && hao_get_f64(o, 1) == 3 && hao_get_f64(o, 4) == 4);
    hao_array_free(o);
    CHECK(hao_grade_down(a5, &o) == 0 && hao_get_f64(o, 0) == 4 && hao_get_f64(o, 3) == 1 && hao_get_f64(o, 4) == 3);
    hao_array_free(o);

    hao_array_free(m); hao_array_free(mt); hao_array_free(cube); hao_array_free(a3);
    hao_array_free(a2); hao_array_free(a5); hao_array_free(empty);
    if (failures) { printf("array ext tests: %d failure(s)\n", failures); return 1; }
    printf("array ext tests passed\n");
    return 0;
}
