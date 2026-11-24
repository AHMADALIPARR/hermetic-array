/*
 * HERMETIC-Ω (hermetic-array) · src/array_ext.c
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
   Structural, broadcasting, reduction, scan, product and grade arrays.
   Every kernel validates rank, shape and type and returns a negative
   code on failure: -1 invalid argument / allocation, -2 length or shape
   mismatch, -3 domain, -4 rank or axis. No partial results escape. */

#include "array_ext.h"
#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

double hao_get_f64(const HaoArray *a, size_t lin) {
    if (!a || lin >= a->element_count) return NAN;
    switch (a->type) {
        case HAO_SCALAR_F64: return ((const double *)a->data)[lin];
        case HAO_SCALAR_I64: return (double)((const int64_t *)a->data)[lin];
        case HAO_SCALAR_BOOL: return ((const bool *)a->data)[lin] ? 1.0 : 0.0;
        default: return NAN;
    }
}

int hao_set_f64(HaoArray *a, size_t lin, double v) {
    if (!a || lin >= a->element_count) return -1;
    switch (a->type) {
        case HAO_SCALAR_F64: ((double *)a->data)[lin] = v; return 0;
        case HAO_SCALAR_I64: ((int64_t *)a->data)[lin] = (int64_t)v; return 0;
        case HAO_SCALAR_BOOL: ((bool *)a->data)[lin] = (v != 0.0); return 0;
        default: return -1;
    }
}

int hao_array_copy(const HaoArray *a, HaoArray **out) {
    if (!a || !out) return -1;
    *out = hao_array_create(a->type, a->rank, a->shape);
    if (!*out) return -1;
    if (a->element_count > 0) memcpy((*out)->data, a->data, a->element_count * hao_array_element_size(a->type));
    return 0;
}

int hao_from_f64(const double *src, size_t rank, const size_t *shape, HaoArray **out) {
    if (!out || (rank > 0 && !shape)) return -1;
    *out = hao_array_create(HAO_SCALAR_F64, rank, shape);
    if (!*out) return -1;
    if ((*out)->element_count > 0) {
        if (!src) { hao_array_free(*out); *out = NULL; return -1; }
        memcpy((*out)->data, src, (*out)->element_count * sizeof(double));
    }
    return 0;
}

static void unravel(size_t lin, size_t rank, const size_t *shape, size_t *idx) {
    for (size_t i = rank; i-- > 0;) {
        idx[i] = shape[i] ? lin % shape[i] : 0;
        lin = shape[i] ? lin / shape[i] : 0;
    }
}

static size_t ravel(size_t rank, const size_t *shape, const size_t *idx) {
    size_t lin = 0;
    for (size_t i = 0; i < rank; i++) lin = lin * shape[i] + idx[i];
    return lin;
}

static void copy_elem(HaoArray *dst, size_t di, const HaoArray *src, size_t si) {
    size_t es = hao_array_element_size(src->type);
    if (dst->type == src->type) {
        memcpy((char *)dst->data + di * es, (const char *)src->data + si * es, es);
    } else {
        hao_set_f64(dst, di, hao_get_f64(src, si));
    }
}

int hao_iota(size_t n, HaoArray **out) {
    if (!out) return -1;
    size_t sh[1] = {n};
    *out = hao_array_create(HAO_SCALAR_I64, 1, sh);
    if (!*out) return -1;
    int64_t *d = (*out)->data;
    for (size_t i = 0; i < n; i++) d[i] = (int64_t)i;
    return 0;
}

int hao_reverse(const HaoArray *a, size_t axis, HaoArray **out) {
    if (!a || !out) return -1;
    if (a->rank == 0) return hao_array_copy(a, out);
    if (axis >= a->rank || a->rank > HAO_MAX_RANK) return -4;
    *out = hao_array_create(a->type, a->rank, a->shape);
    if (!*out) return -1;
    size_t idx[HAO_MAX_RANK];
    for (size_t lin = 0; lin < a->element_count; lin++) {
        unravel(lin, a->rank, a->shape, idx);
        idx[axis] = a->shape[axis] - 1 - idx[axis];
        copy_elem(*out, ravel(a->rank, a->shape, idx), a, lin);
    }
    return 0;
}

/* Result axis i is source axis perm[i]: shape'[i] = shape[perm[i]]. */
int hao_transpose_axes(const HaoArray *a, const size_t *perm, HaoArray **out) {
    if (!a || !perm || !out) return -1;
    if (a->rank > HAO_MAX_RANK) return -4;
    bool seen[HAO_MAX_RANK] = {0};
    for (size_t i = 0; i < a->rank; i++) {
        if (perm[i] >= a->rank || seen[perm[i]]) return -4;
        seen[perm[i]] = true;
    }
    size_t new_shape[HAO_MAX_RANK];
    for (size_t i = 0; i < a->rank; i++) new_shape[i] = a->shape[perm[i]];
    *out = hao_array_create(a->type, a->rank, new_shape);
    if (!*out) return -1;
    size_t idx[HAO_MAX_RANK], nidx[HAO_MAX_RANK];
    for (size_t lin = 0; lin < a->element_count; lin++) {
        unravel(lin, a->rank, a->shape, idx);
        for (size_t i = 0; i < a->rank; i++) nidx[i] = idx[perm[i]];
        copy_elem(*out, ravel(a->rank, new_shape, nidx), a, lin);
    }
    return 0;
}

int hao_transpose(const HaoArray *a, HaoArray **out) {
    if (!a || !out) return -1;
    if (a->rank > HAO_MAX_RANK) return -4;
    size_t perm[HAO_MAX_RANK];
    for (size_t i = 0; i < a->rank; i++) perm[i] = a->rank - 1 - i;
    return hao_transpose_axes(a, perm, out);
}

/* APL-style reshape: under-fill truncates, over-fill cycles the ravel.
   An empty source with a non-empty target is a length error. */
int hao_reshape_fill(const HaoArray *a, size_t rank, const size_t *shape, HaoArray **out) {
    if (!a || !out || (rank > 0 && !shape)) return -1;
    size_t count = 1;
    for (size_t i = 0; i < rank; i++) count *= shape[i];
    if (count > 0 && a->element_count == 0) return -2;
    *out = hao_array_create(a->type, rank, shape);
    if (!*out) return -1;
    for (size_t i = 0; i < count; i++) copy_elem(*out, i, a, i % a->element_count);
    return 0;
}

static size_t major_cell_size(const HaoArray *a) {
    size_t c = 1;
    for (size_t i = 1; i < a->rank; i++) c *= a->shape[i];
    return c;
}

/* Take n major cells (n < 0 takes from the end). Over-take is a length error. */
int hao_take(const HaoArray *a, int64_t n, HaoArray **out) {
    if (!a || !out || a->rank == 0 || a->rank > HAO_MAX_RANK) return -1;
    size_t len = a->shape[0];
    size_t k = (size_t)(n < 0 ? -n : n);
    if (k > len) return -2;
    size_t sh[HAO_MAX_RANK];
    memcpy(sh, a->shape, a->rank * sizeof(size_t));
    sh[0] = k;
    *out = hao_array_create(a->type, a->rank, sh);
    if (!*out) return -1;
    size_t cell = major_cell_size(a);
    size_t start = (n < 0) ? (len - k) : 0;
    for (size_t i = 0; i < k * cell; i++) copy_elem(*out, i, a, start * cell + i);
    return 0;
}

int hao_drop(const HaoArray *a, int64_t n, HaoArray **out) {
    if (!a || !out || a->rank == 0 || a->rank > HAO_MAX_RANK) return -1;
    size_t len = a->shape[0];
    size_t k = (size_t)(n < 0 ? -n : n);
    if (k > len) k = len;
    int64_t keep = (int64_t)(len - k);
    return hao_take(a, n < 0 ? keep : -keep, out);
}

/* Join along the leading axis; trailing shapes must agree. */
int hao_join(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (!a || !b || !out) return -1;
    if (a->rank == 0 || b->rank == 0 || a->rank != b->rank || a->rank > HAO_MAX_RANK) return -4;
    for (size_t i = 1; i < a->rank; i++) if (a->shape[i] != b->shape[i]) return -2;
    size_t sh[HAO_MAX_RANK];
    memcpy(sh, a->shape, a->rank * sizeof(size_t));
    sh[0] = a->shape[0] + b->shape[0];
    HaoScalarType t = (a->type == b->type) ? a->type : HAO_SCALAR_F64;
    *out = hao_array_create(t, a->rank, sh);
    if (!*out) return -1;
    for (size_t i = 0; i < a->element_count; i++) copy_elem(*out, i, a, i);
    for (size_t i = 0; i < b->element_count; i++) copy_elem(*out, a->element_count + i, b, i);
    return 0;
}

static int broadcast_shape(size_t ra, const size_t *sa, size_t rb, const size_t *sb, size_t *rank, size_t *shape) {
    size_t r = ra > rb ? ra : rb;
    if (r > HAO_MAX_RANK) return -4;
    for (size_t i = 0; i < r; i++) {
        size_t da = (i < r - ra) ? 1 : sa[i - (r - ra)];
        size_t db = (i < r - rb) ? 1 : sb[i - (r - rb)];
        if (da != db && da != 1 && db != 1) return -2;
        shape[i] = (da == 1) ? db : da;
    }
    *rank = r;
    return 0;
}

static size_t broadcast_source(const HaoArray *a, size_t r, const size_t *idx) {
    size_t lin = 0;
    for (size_t i = 0; i < a->rank; i++) {
        size_t oi = idx[i + (r - a->rank)];
        lin = lin * a->shape[i] + (a->shape[i] == 1 ? 0 : oi);
    }
    return lin;
}

int hao_broadcast(const HaoArray *a, size_t rank, const size_t *shape, HaoArray **out) {
    if (!a || !out || (rank > 0 && !shape) || rank > HAO_MAX_RANK || a->rank > rank) return -1;
    size_t r, sh[HAO_MAX_RANK];
    int rc = broadcast_shape(a->rank, a->shape, rank, shape, &r, sh);
    if (rc) return rc;
    for (size_t i = 0; i < rank; i++) if (sh[i] != shape[i]) return -2;
    *out = hao_array_create(a->type, rank, shape);
    if (!*out) return -1;
    size_t idx[HAO_MAX_RANK];
    for (size_t lin = 0; lin < (*out)->element_count; lin++) {
        unravel(lin, rank, shape, idx);
        copy_elem(*out, lin, a, broadcast_source(a, rank, idx));
    }
    return 0;
}

static double apply_bin(HaoBinOp op, double x, double y) {
    switch (op) {
        case HAO_BIN_ADD: return x + y;
        case HAO_BIN_SUB: return x - y;
        case HAO_BIN_MUL: return x * y;
        case HAO_BIN_DIV: return x / y;
        case HAO_BIN_MAX: return x > y ? x : y;
        case HAO_BIN_MIN: return x < y ? x : y;
        case HAO_BIN_POW: return pow(x, y);
        default: return NAN;
    }
}

int hao_map2(const HaoArray *a, const HaoArray *b, HaoBinOp op, HaoArray **out) {
    if (!a || !b || !out) return -1;
    size_t r, sh[HAO_MAX_RANK];
    int rc = broadcast_shape(a->rank, a->shape, b->rank, b->shape, &r, sh);
    if (rc) return rc;
    *out = hao_array_create(HAO_SCALAR_F64, r, sh);
    if (!*out) return -1;
    double *od = (*out)->data;
    size_t n = (*out)->element_count;
    if (a->type == HAO_SCALAR_F64 && b->type == HAO_SCALAR_F64 &&
        a->element_count == n && b->element_count == n && a->rank == b->rank) {
        const double *ad = a->data, *bd = b->data;
        size_t i = 0;
        for (; i + 3 < n; i += 4) {
            od[i]   = apply_bin(op, ad[i],   bd[i]);
            od[i+1] = apply_bin(op, ad[i+1], bd[i+1]);
            od[i+2] = apply_bin(op, ad[i+2], bd[i+2]);
            od[i+3] = apply_bin(op, ad[i+3], bd[i+3]);
        }
        for (; i < n; i++) od[i] = apply_bin(op, ad[i], bd[i]);
        return 0;
    }
    size_t idx[HAO_MAX_RANK];
    for (size_t lin = 0; lin < n; lin++) {
        unravel(lin, r, sh, idx);
        od[lin] = apply_bin(op, hao_get_f64(a, broadcast_source(a, r, idx)),
                                hao_get_f64(b, broadcast_source(b, r, idx)));
    }
    return 0;
}

int hao_scale(const HaoArray *a, double s, HaoArray **out) {
    if (!a || !out) return -1;
    *out = hao_array_create(HAO_SCALAR_F64, a->rank, a->shape);
    if (!*out) return -1;
    double *od = (*out)->data;
    for (size_t i = 0; i < a->element_count; i++) od[i] = s * hao_get_f64(a, i);
    return 0;
}

static int reduce_or_scan(const HaoArray *a, size_t axis, HaoBinOp op, bool scan, HaoArray **out) {
    if (!a || !out) return -1;
    if (a->rank == 0 || axis >= a->rank || a->rank > HAO_MAX_RANK) return -4;
    size_t len = a->shape[axis];
    if (len == 0 && !scan && op != HAO_BIN_ADD && op != HAO_BIN_MUL) return -3;
    size_t sh[HAO_MAX_RANK], r = 0;
    for (size_t i = 0; i < a->rank; i++) {
        if (scan || i != axis) sh[r++] = a->shape[i];
    }
    *out = hao_array_create(HAO_SCALAR_F64, r, sh);
    if (!*out) return -1;
    double *od = (*out)->data;
    size_t inner = 1, outer = 1;
    for (size_t i = axis + 1; i < a->rank; i++) inner *= a->shape[i];
    for (size_t i = 0; i < axis; i++) outer *= a->shape[i];
    for (size_t o = 0; o < outer; o++) {
        for (size_t in = 0; in < inner; in++) {
            double acc = (op == HAO_BIN_MUL) ? 1.0 : 0.0;
            for (size_t k = 0; k < len; k++) {
                double v = hao_get_f64(a, (o * len + k) * inner + in);
                acc = (k == 0) ? v : apply_bin(op, acc, v);
                if (scan) od[(o * len + k) * inner + in] = acc;
            }
            if (!scan) od[o * inner + in] = acc;
        }
    }
    return 0;
}

int hao_reduce(const HaoArray *a, size_t axis, HaoBinOp op, HaoArray **out) {
    return reduce_or_scan(a, axis, op, false, out);
}

int hao_scan(const HaoArray *a, size_t axis, HaoBinOp op, HaoArray **out) {
    return reduce_or_scan(a, axis, op, true, out);
}

/* Tensor (outer) product ⊗: shape is the concatenation of both shapes. */
int hao_outer(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (!a || !b || !out) return -1;
    if (a->rank + b->rank > HAO_MAX_RANK) return -4;
    size_t sh[HAO_MAX_RANK];
    if (a->rank) memcpy(sh, a->shape, a->rank * sizeof(size_t));
    if (b->rank) memcpy(sh + a->rank, b->shape, b->rank * sizeof(size_t));
    *out = hao_array_create(HAO_SCALAR_F64, a->rank + b->rank, sh);
    if (!*out) return -1;
    double *od = (*out)->data;
    size_t nb = b->element_count;
    for (size_t i = 0; i < a->element_count; i++) {
        double av = hao_get_f64(a, i);
        for (size_t j = 0; j < nb; j++) od[i * nb + j] = av * hao_get_f64(b, j);
    }
    return 0;
}

/* Inner product of an m×k and a k×n matrix (vectors are promoted). */
int hao_matmul(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (!a || !b || !out) return -1;
    if (a->rank < 1 || a->rank > 2 || b->rank < 1 || b->rank > 2) return -4;
    size_t m = (a->rank == 2) ? a->shape[0] : 1;
    size_t k = a->shape[a->rank - 1];
    size_t kb = b->shape[0];
    size_t n = (b->rank == 2) ? b->shape[1] : 1;
    if (k != kb) return -2;
    size_t sh[2], r = 0;
    if (a->rank == 2) sh[r++] = m;
    if (b->rank == 2) sh[r++] = n;
    *out = hao_array_create(HAO_SCALAR_F64, r, sh);
    if (!*out) return -1;
    double *od = (*out)->data;
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            double s = 0.0;
            for (size_t t = 0; t < k; t++) s += hao_get_f64(a, i * k + t) * hao_get_f64(b, t * n + j);
            od[i * n + j] = s;
        }
    }
    return 0;
}

/* Generalized contraction: trace over axes i and j (equal lengths). */
int hao_contract(const HaoArray *a, size_t axis_i, size_t axis_j, HaoArray **out) {
    if (!a || !out) return -1;
    if (a->rank < 2 || a->rank > HAO_MAX_RANK || axis_i >= a->rank || axis_j >= a->rank || axis_i == axis_j) return -4;
    if (a->shape[axis_i] != a->shape[axis_j]) return -2;
    size_t sh[HAO_MAX_RANK], r = 0;
    for (size_t i = 0; i < a->rank; i++) if (i != axis_i && i != axis_j) sh[r++] = a->shape[i];
    *out = hao_array_create(HAO_SCALAR_F64, r, sh);
    if (!*out) return -1;
    double *od = (*out)->data;
    for (size_t i = 0; i < (*out)->element_count; i++) od[i] = 0.0;
    size_t idx[HAO_MAX_RANK], oidx[HAO_MAX_RANK];
    for (size_t lin = 0; lin < a->element_count; lin++) {
        unravel(lin, a->rank, a->shape, idx);
        if (idx[axis_i] != idx[axis_j]) continue;
        size_t q = 0;
        for (size_t i = 0; i < a->rank; i++) if (i != axis_i && i != axis_j) oidx[q++] = idx[i];
        od[ravel(r, sh, oidx)] += hao_get_f64(a, lin);
    }
    return 0;
}

static int grade(const HaoArray *a, bool down, HaoArray **out) {
    if (!a || !out) return -1;
    if (a->rank != 1) return -4;
    size_t n = a->element_count;
    size_t sh[1] = {n};
    *out = hao_array_create(HAO_SCALAR_I64, 1, sh);
    if (!*out) return -1;
    int64_t *g = (*out)->data;
    for (size_t i = 0; i < n; i++) g[i] = (int64_t)i;
    /* stable insertion sort keeps ties in index order */
    for (size_t i = 1; i < n; i++) {
        int64_t cur = g[i];
        double cv = hao_get_f64(a, (size_t)cur);
        size_t j = i;
        while (j > 0) {
            double pv = hao_get_f64(a, (size_t)g[j - 1]);
            bool after = down ? (pv < cv) : (pv > cv);
            if (!after) break;
            g[j] = g[j - 1];
            j--;
        }
        g[j] = cur;
    }
    return 0;
}

int hao_grade_up(const HaoArray *a, HaoArray **out) { return grade(a, false, out); }
int hao_grade_down(const HaoArray *a, HaoArray **out) { return grade(a, true, out); }
