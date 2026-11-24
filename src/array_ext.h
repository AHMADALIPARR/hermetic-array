/*
 * HERMETIC-Ω (hermetic-array) · src/array_ext.h
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

/* HERMETIC-Ω array engine extensions (additions only).
   Completes the structural, reduction, broadcasting and product arrays
   listed in docs/operators/README.md and lang/tensor.apl / lang/engine.apl
   that had no C11 kernel. Original kernels in array.c are untouched. */

#ifndef HAO_ARRAY_EXT_H
#define HAO_ARRAY_EXT_H

#include "array.h"

#define HAO_MAX_RANK 32

typedef enum {
    HAO_BIN_ADD,
    HAO_BIN_SUB,
    HAO_BIN_MUL,
    HAO_BIN_DIV,
    HAO_BIN_MAX,
    HAO_BIN_MIN,
    HAO_BIN_POW
} HaoBinOp;

double hao_get_f64(const HaoArray *a, size_t lin);
int hao_set_f64(HaoArray *a, size_t lin, double v);
int hao_array_copy(const HaoArray *a, HaoArray **out);
int hao_from_f64(const double *src, size_t rank, const size_t *shape, HaoArray **out);

/* Structural geometry */
int hao_iota(size_t n, HaoArray **out);
int hao_reverse(const HaoArray *a, size_t axis, HaoArray **out);
int hao_transpose(const HaoArray *a, HaoArray **out);
int hao_transpose_axes(const HaoArray *a, const size_t *perm, HaoArray **out);
int hao_reshape_fill(const HaoArray *a, size_t rank, const size_t *shape, HaoArray **out);
int hao_take(const HaoArray *a, int64_t n, HaoArray **out);
int hao_drop(const HaoArray *a, int64_t n, HaoArray **out);
int hao_join(const HaoArray *a, const HaoArray *b, HaoArray **out);
int hao_broadcast(const HaoArray *a, size_t rank, const size_t *shape, HaoArray **out);

/* Pervasive arithmetic with broadcasting */
int hao_map2(const HaoArray *a, const HaoArray *b, HaoBinOp op, HaoArray **out);
int hao_scale(const HaoArray *a, double s, HaoArray **out);

/* Reduction and scan along an axis */
int hao_reduce(const HaoArray *a, size_t axis, HaoBinOp op, HaoArray **out);
int hao_scan(const HaoArray *a, size_t axis, HaoBinOp op, HaoArray **out);

/* Products and contraction */
int hao_outer(const HaoArray *a, const HaoArray *b, HaoArray **out);
int hao_matmul(const HaoArray *a, const HaoArray *b, HaoArray **out);
int hao_contract(const HaoArray *a, size_t axis_i, size_t axis_j, HaoArray **out);

/* Grade */
int hao_grade_up(const HaoArray *a, HaoArray **out);
int hao_grade_down(const HaoArray *a, HaoArray **out);

#endif
