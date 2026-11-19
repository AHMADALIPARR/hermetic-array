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
} HaoArray;

HaoArray *hao_array_create(HaoScalarType type, size_t rank, const size_t *shape);
void hao_array_free(HaoArray *a);
size_t hao_array_element_size(HaoScalarType t);
void *hao_array_ptr(HaoArray *a, const size_t *indices);
bool hao_array_check_bounds(const HaoArray *a, const size_t *indices);

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

#endif
