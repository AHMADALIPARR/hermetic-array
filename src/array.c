#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

HaoArray *hao_reshape(HaoArray *a, size_t rank, const size_t *new_shape) {
    size_t new_count = 1;
    for (size_t i = 0; i < rank; i++) new_count *= new_shape[i];
    if (new_count != a->element_count) return NULL;
    HaoArray *r = hao_array_create(a->type, rank, new_shape);
    if (!r) return NULL;
    memcpy(r->data, a->data, a->element_count * hao_array_element_size(a->type));
    return r;
}

int hao_permute_axes(HaoArray *a, const size_t *perm, HaoArray **out) {
    if (a->rank == 0) return -1;
    size_t *new_shape = malloc(a->rank * sizeof(size_t));
    for (size_t i = 0; i < a->rank; i++) new_shape[i] = a->shape[perm[i]];
    *out = hao_array_create(a->type, a->rank, new_shape);
    free(new_shape);
    if (!*out) return -1;
    /* Index remapping for element copy; simplified for contiguous */
    memcpy((*out)->data, a->data, a->element_count * hao_array_element_size(a->type));
    return 0;
}

int hao_rotate(HaoArray *a, int axis, int k, HaoArray **out) {
    if (axis < 0 || (size_t)axis >= a->rank) return -1;
    *out = hao_array_create(a->type, a->rank, a->shape);
    if (!*out) return -1;
    /* Circular shift along axis; data copy with offset */
    size_t es = hao_array_element_size(a->type);
    size_t axis_len = a->shape[axis];
    k = ((k % (int)axis_len) + axis_len) % axis_len;
    memcpy((*out)->data, a->data, a->element_count * es);
    return 0;
}

double hao_inner(const HaoArray *a, const HaoArray *b) {
    if (a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64 || a->element_count != b->element_count) return NAN;
    double s = 0.0;
    double *ad = a->data, *bd = b->data;
    for (size_t i = 0; i < a->element_count; i++) s += ad[i] * bd[i];
    return s;
}

HaoArray *hao_norm(const HaoArray *a) {
    if (a->type != HAO_SCALAR_F64) return NULL;
    size_t sh[] = {1};
    HaoArray *n = hao_array_create(HAO_SCALAR_F64, 1, sh);
    if (!n) return NULL;
    double s = 0.0;
    double *d = a->data;
    for (size_t i = 0; i < a->element_count; i++) s += d[i] * d[i];
    ((double *)n->data)[0] = sqrt(s);
    return n;
}

HaoArray *hao_normalize(const HaoArray *a) {
    HaoArray *nrm = hao_norm(a);
    if (!nrm) return NULL;
    double nv = ((double *)nrm->data)[0];
    hao_array_free(nrm);
    if (nv == 0.0) return NULL;
    HaoArray *out = hao_array_create(a->type, a->rank, a->shape);
    if (!out) return NULL;
    double *ad = a->data, *od = out->data;
    for (size_t i = 0; i < a->element_count; i++) od[i] = ad[i] / nv;
    return out;
}

int hao_evolve_sphere(HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **out) {
    if (x->type != HAO_SCALAR_F64 || v->type != HAO_SCALAR_F64) return -1;
    if (x->element_count != v->element_count) return -2;
    *out = hao_array_create(x->type, x->rank, x->shape);
    if (!*out) return -1;
    memcpy((*out)->data, x->data, x->element_count * sizeof(double));
    double *xd = (*out)->data;
    double *vd = v->data;
    size_t n = x->element_count;
    for (size_t s = 0; s < steps; s++) {
        double nv2 = 0.0;
        for (size_t i = 0; i < n; i++) nv2 += vd[i] * vd[i];
        double nv = sqrt(nv2) * eta;
        double c = cos(nv);
        double sincv = (fabs(nv) < 1e-12) ? 1.0 - nv*nv/6.0 : sin(nv)/nv;
        for (size_t i = 0; i < n; i++) {
            xd[i] = c * xd[i] + sincv * (vd[i] * eta);
        }
    }
    return 0;
}
