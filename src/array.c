#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

size_t hao_array_element_size(HaoScalarType t) {
    switch (t) {
        case HAO_SCALAR_F64: return sizeof(double);
        case HAO_SCALAR_I64: return sizeof(int64_t);
        case HAO_SCALAR_BOOL: return sizeof(bool);
        default: return 0;
    }
}

HaoArray *hao_array_create(HaoScalarType type, size_t rank, const size_t *shape) {
    HaoArray *a = calloc(1, sizeof(HaoArray));
    if (!a) return NULL;
    a->type = type;
    a->rank = rank;
    a->owned = true;
    a->shape = malloc(rank * sizeof(size_t));
    a->strides = malloc(rank * sizeof(size_t));
    if (!a->shape || !a->strides) { hao_array_free(a); return NULL; }
    memcpy(a->shape, shape, rank * sizeof(size_t));
    size_t es = hao_array_element_size(type);
    size_t count = 1;
    for (size_t i = 0; i < rank; i++) count *= shape[i];
    a->element_count = count;
    a->data = calloc(count, es);
    if (!a->data && count > 0) { hao_array_free(a); return NULL; }
    size_t stride = 1;
    for (int i = (int)rank - 1; i >= 0; i--) {
        a->strides[i] = stride;
        stride *= shape[i];
    }
    return a;
}

void hao_array_free(HaoArray *a) {
    if (!a) return;
    if (a->owned) {
        free(a->shape);
        free(a->strides);
        free(a->data);
    }
    free(a);
}

void *hao_array_ptr(HaoArray *a, const size_t *indices) {
    if (!hao_array_check_bounds(a, indices)) return NULL;
    size_t offset = 0;
    for (size_t i = 0; i < a->rank; i++) offset += indices[i] * a->strides[i];
    return (char *)a->data + offset * hao_array_element_size(a->type);
}

bool hao_array_check_bounds(const HaoArray *a, const size_t *indices) {
    for (size_t i = 0; i < a->rank; i++) {
        if (indices[i] >= a->shape[i]) return false;
    }
    return true;
}
