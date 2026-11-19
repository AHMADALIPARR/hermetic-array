#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

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
    if (rank > 0) {
        a->shape = malloc(rank * sizeof(size_t));
        a->strides = malloc(rank * sizeof(size_t));
        if (!a->shape || !a->strides) { hao_array_free(a); return NULL; }
        memcpy(a->shape, shape, rank * sizeof(size_t));
    }
    size_t es = hao_array_element_size(type);
    size_t count = 1;
    for (size_t i = 0; i < rank; i++) {
        if (shape[i] == 0) { count = 0; break; }
        count *= shape[i];
    }
    a->element_count = count;
    if (count > 0) {
        a->data = calloc(count, es);
        if (!a->data) { hao_array_free(a); return NULL; }
    }
    if (rank > 0) {
        size_t stride = 1;
        for (int i = (int)rank - 1; i >= 0; i--) {
            a->strides[i] = stride;
            stride *= shape[i];
        }
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
    if (!a || !indices) return false;
    for (size_t i = 0; i < a->rank; i++) {
        if (indices[i] >= a->shape[i]) return false;
    }
    return true;
}

HaoArray *hao_reshape(HaoArray *a, size_t rank, const size_t *new_shape) {
    if (!a) return NULL;
    size_t new_count = 1;
    for (size_t i = 0; i < rank; i++) new_count *= new_shape[i];
    if (new_count != a->element_count) return NULL; /* length error */
    HaoArray *r = hao_array_create(a->type, rank, new_shape);
    if (!r) return NULL;
    if (a->element_count > 0)
        memcpy(r->data, a->data, a->element_count * hao_array_element_size(a->type));
    return r;
}

static size_t compute_offset(const HaoArray *a, const size_t *idx) {
    size_t off = 0;
    for (size_t i = 0; i < a->rank; i++) off += idx[i] * a->strides[i];
    return off;
}

int hao_permute_axes(HaoArray *a, const size_t *perm, HaoArray **out) {
    if (!a || !perm || a->rank == 0) return -1;
    /* Validate perm is a permutation */
    bool seen[32] = {0};
    if (a->rank > 32) return -1;
    for (size_t i = 0; i < a->rank; i++) {
        if (perm[i] >= a->rank || seen[perm[i]]) return -1;
        seen[perm[i]] = true;
    }
    size_t *new_shape = malloc(a->rank * sizeof(size_t));
    for (size_t i = 0; i < a->rank; i++) new_shape[i] = a->shape[perm[i]];
    *out = hao_array_create(a->type, a->rank, new_shape);
    free(new_shape);
    if (!*out) return -1;
    if (a->element_count == 0) return 0;
    size_t es = hao_array_element_size(a->type);
    size_t *idx = calloc(a->rank, sizeof(size_t));
    size_t *new_idx = calloc(a->rank, sizeof(size_t));
    for (size_t lin = 0; lin < a->element_count; lin++) {
        /* Convert linear to multi-index for original */
        size_t rem = lin;
        for (int i = (int)a->rank - 1; i >= 0; i--) {
            idx[i] = rem % a->shape[i];
            rem /= a->shape[i];
        }
        /* Map to new multi-index via inverse perm */
        for (size_t i = 0; i < a->rank; i++) new_idx[perm[i]] = idx[i];
        size_t new_off = compute_offset(*out, new_idx);
        memcpy((char *)(*out)->data + new_off * es, (char *)a->data + lin * es, es);
    }
    free(idx); free(new_idx);
    return 0;
}

int hao_rotate(HaoArray *a, int axis, int k, HaoArray **out) {
    if (!a || axis < 0 || (size_t)axis >= a->rank) return -1;
    *out = hao_array_create(a->type, a->rank, a->shape);
    if (!*out) return -1;
    if (a->element_count == 0) return 0;
    size_t axis_len = a->shape[axis];
    int shift = ((k % (int)axis_len) + (int)axis_len) % (int)axis_len;
    size_t es = hao_array_element_size(a->type);
    size_t *idx = calloc(a->rank, sizeof(size_t));
    for (size_t lin = 0; lin < a->element_count; lin++) {
        size_t rem = lin;
        for (int i = (int)a->rank - 1; i >= 0; i--) {
            idx[i] = rem % a->shape[i];
            rem /= a->shape[i];
        }
        idx[axis] = (idx[axis] + shift) % axis_len;
        size_t new_off = compute_offset(*out, idx);
        memcpy((char *)(*out)->data + new_off * es, (char *)a->data + lin * es, es);
    }
    free(idx);
    return 0;
}

double hao_inner(const HaoArray *a, const HaoArray *b) {
    if (!a || !b || a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64 || a->element_count != b->element_count) return NAN;
    double s = 0.0;
    const double *ad = a->data, *bd = b->data;
    for (size_t i = 0; i < a->element_count; i++) s += ad[i] * bd[i];
    return s;
}

HaoArray *hao_norm(const HaoArray *a) {
    if (!a || a->type != HAO_SCALAR_F64) return NULL;
    size_t sh[] = {1};
    HaoArray *n = hao_array_create(HAO_SCALAR_F64, 1, sh);
    if (!n) return NULL;
    double s = 0.0;
    const double *d = a->data;
    for (size_t i = 0; i < a->element_count; i++) s += d[i] * d[i];
    ((double *)n->data)[0] = sqrt(s);
    return n;
}

HaoArray *hao_normalize(const HaoArray *a) {
    if (!a) return NULL;
    HaoArray *nrm = hao_norm(a);
    if (!nrm) return NULL;
    double nv = ((double *)nrm->data)[0];
    hao_array_free(nrm);
    if (nv == 0.0) return NULL;
    HaoArray *out = hao_array_create(a->type, a->rank, a->shape);
    if (!out) return NULL;
    const double *ad = a->data;
    double *od = out->data;
    for (size_t i = 0; i < a->element_count; i++) od[i] = ad[i] / nv;
    return out;
}

int hao_evolve_sphere(HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **out) {
    if (!x || !v || x->type != HAO_SCALAR_F64 || v->type != HAO_SCALAR_F64) return -1;
    if (x->element_count != v->element_count) return -2;
    /* Precondition checks */
    double xn2 = 0.0, dot = 0.0;
    const double *xd0 = x->data, *vd0 = v->data;
    for (size_t i = 0; i < x->element_count; i++) {
        xn2 += xd0[i] * xd0[i];
        dot += xd0[i] * vd0[i];
    }
    if (fabs(xn2 - 1.0) > 1e-8 || fabs(dot) > 1e-8) return -3; /* not unit or not tangent */
    *out = hao_array_create(x->type, x->rank, x->shape);
    if (!*out) return -1;
    memcpy((*out)->data, x->data, x->element_count * sizeof(double));
    double *xd = (*out)->data;
    const double *vd = v->data;
    size_t n = x->element_count;
    for (size_t s = 0; s < steps; s++) {
        double nv2 = 0.0;
        for (size_t i = 0; i < n; i++) nv2 += vd[i] * vd[i];
        double nv = sqrt(nv2) * fabs(eta);
        double c = cos(nv);
        double sincv = (fabs(nv) < 1e-12) ? 1.0 - nv*nv/6.0 : sin(nv)/nv;
        for (size_t i = 0; i < n; i++) {
            xd[i] = c * xd[i] + sincv * (vd[i] * eta);
        }
    }
    return 0;
}
