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
    size_t *shape;      /* owned */
    size_t *strides;    /* owned, in elements */
    void *data;         /* owned contiguous buffer */
    size_t element_count;
    bool owned;
} HaoArray;

HaoArray *hao_array_create(HaoScalarType type, size_t rank, const size_t *shape);
void hao_array_free(HaoArray *a);
HaoArray *hao_array_view(HaoArray *base, size_t rank, const size_t *shape, const size_t *strides, void *data);
size_t hao_array_element_size(HaoScalarType t);
void *hao_array_ptr(HaoArray *a, const size_t *indices);
bool hao_array_check_bounds(const HaoArray *a, const size_t *indices);

#endif
