#ifndef HAO_ARRAY_H
#define HAO_ARRAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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
    size_t bytes;          /* total allocated bytes for data */
    size_t alloc_count;    /* number of live allocations tracked */
} HaoArray;

/* Global memory accounting (zero-dep, atomic-free for single-thread) */
extern size_t hao_total_bytes;
extern size_t hao_peak_bytes;
extern size_t hao_alloc_count;

HaoArray *hao_array_create(HaoScalarType type, size_t rank, const size_t *shape);
void hao_array_free(HaoArray *a);
size_t hao_array_element_size(HaoScalarType t);
void *hao_array_ptr(HaoArray *a, const size_t *indices);
bool hao_array_check_bounds(const HaoArray *a, const size_t *indices);

/* Fast pointer arithmetic offset */
static inline size_t hao_offset(const HaoArray *a, const size_t *idx) {
    size_t off = 0;
    const size_t *s = a->strides;
    for (size_t i = 0; i < a->rank; i++) off += idx[i] * s[i];
    return off;
}

/* Structural */
HaoArray *hao_reshape(HaoArray *a, size_t rank, const size_t *new_shape);
int hao_permute_axes(HaoArray *a, const size_t *perm, HaoArray **out);
int hao_rotate(HaoArray *a, int axis, int k, HaoArray **out);

/* Metric */
double hao_inner(const HaoArray *a, const HaoArray *b);
HaoArray *hao_norm(const HaoArray *a);
HaoArray *hao_normalize(const HaoArray *a);

/* Evolution */
int hao_evolve_sphere(HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **out);

/* Memory report */
void hao_memory_report(void);

#endif
