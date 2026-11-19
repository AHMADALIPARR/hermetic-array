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

#ifndef HAO_ARRAY_H
#define HAO_ARRAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

#define HAO_CACHE_LINE 64

typedef enum {
    HAO_SCALAR_F64,
    HAO_SCALAR_I64,
    HAO_SCALAR_BOOL
} HaoScalarType;

typedef struct {
    HaoScalarType type;
    size_t rank;
    size_t *shape;
    size_t *strides;
    void *data;
    size_t element_count;
    bool owned;
    size_t bytes;
    size_t alloc_count;
} HaoArray;

typedef struct {
    alignas(HAO_CACHE_LINE) char *base;
    size_t capacity;
    size_t used;
    size_t peak;
} HaoPool;

extern HaoPool hao_pool;
extern size_t hao_total_bytes;
extern size_t hao_peak_bytes;
extern size_t hao_alloc_count;

void hao_pool_init(size_t capacity);
void *hao_pool_alloc(size_t bytes);
void hao_pool_reset(void);
void hao_pool_free(void);

HaoArray *hao_array_create(HaoScalarType type, size_t rank, const size_t *shape);
void hao_array_free(HaoArray *a);
size_t hao_array_element_size(HaoScalarType t);
void *hao_array_ptr(HaoArray *a, const size_t *indices);
bool hao_array_check_bounds(const HaoArray *a, const size_t *indices);

static inline size_t hao_offset(const HaoArray *a, const size_t *idx) {
    size_t off = 0;
    const size_t *s = a->strides;
    for (size_t i = 0; i < a->rank; i++) off += idx[i] * s[i];
    return off;
}

static inline void hao_vec4_add_mul(double *out, const double *a, const double *b, double s, size_t n) {
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        out[i]   = a[i]   + s * b[i];
        out[i+1] = a[i+1] + s * b[i+1];
        out[i+2] = a[i+2] + s * b[i+2];
        out[i+3] = a[i+3] + s * b[i+3];
    }
    for (; i < n; i++) out[i] = a[i] + s * b[i];
}

HaoArray *hao_reshape(HaoArray *a, size_t rank, const size_t *new_shape);
int hao_permute_axes(HaoArray *a, const size_t *perm, HaoArray **out);
int hao_rotate(HaoArray *a, int axis, int k, HaoArray **out);

double hao_inner(const HaoArray *a, const HaoArray *b);
HaoArray *hao_norm(const HaoArray *a);
HaoArray *hao_normalize(const HaoArray *a);

int hao_evolve_sphere(HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **out);

void hao_memory_report(void);

#endif
