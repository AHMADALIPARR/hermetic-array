#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

size_t hao_total_bytes = 0;
size_t hao_peak_bytes = 0;
size_t hao_alloc_count = 0;
HaoPool hao_pool = {0};

void hao_pool_init(size_t capacity) {
    if (hao_pool.base) free(hao_pool.base);
    hao_pool.base = malloc(capacity);
    hao_pool.capacity = capacity;
    hao_pool.used = 0;
    hao_pool.peak = 0;
    if (hao_pool.base) {
        hao_total_bytes += capacity;
        hao_alloc_count++;
        if (hao_total_bytes > hao_peak_bytes) hao_peak_bytes = hao_total_bytes;
    }
}

void *hao_pool_alloc(size_t bytes) {
    if (!hao_pool.base || hao_pool.used + bytes > hao_pool.capacity) return NULL;
    void *p = hao_pool.base + hao_pool.used;
    hao_pool.used += bytes;
    if (hao_pool.used > hao_pool.peak) hao_pool.peak = hao_pool.used;
    return p;
}

void hao_pool_reset(void) {
    hao_pool.used = 0;
}

void hao_pool_free(void) {
    if (hao_pool.base) {
        hao_total_bytes -= hao_pool.capacity;
        hao_alloc_count--;
        free(hao_pool.base);
        hao_pool.base = NULL;
        hao_pool.capacity = 0;
        hao_pool.used = 0;
    }
}

static void account_alloc(size_t bytes) {
    hao_total_bytes += bytes;
    hao_alloc_count++;
    if (hao_total_bytes > hao_peak_bytes) hao_peak_bytes = hao_total_bytes;
}

static void account_free(size_t bytes) {
    if (hao_total_bytes >= bytes) hao_total_bytes -= bytes;
    if (hao_alloc_count > 0) hao_alloc_count--;
}

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
    account_alloc(sizeof(HaoArray));
    a->type = type;
    a->rank = rank;
    a->owned = true;
    if (rank > 0) {
        a->shape = malloc(rank * sizeof(size_t));
        a->strides = malloc(rank * sizeof(size_t));
        if (!a->shape || !a->strides) { hao_array_free(a); return NULL; }
        account_alloc(rank * sizeof(size_t) * 2);
        memcpy(a->shape, shape, rank * sizeof(size_t));
    }
    size_t es = hao_array_element_size(type);
    size_t count = 1;
    for (size_t i = 0; i < rank; i++) {
        if (shape[i] == 0) { count = 0; break; }
        count *= shape[i];
    }
    a->element_count = count;
    a->bytes = count * es;
    if (count > 0) {
        a->data = hao_pool_alloc(a->bytes);
        if (!a->data) a->data = calloc(count, es);
        if (!a->data) { hao_array_free(a); return NULL; }
        if (a->data != hao_pool.base + (hao_pool.used - a->bytes)) account_alloc(a->bytes);
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
        if (a->shape) { account_free(a->rank * sizeof(size_t)); free(a->shape); }
        if (a->strides) { account_free(a->rank * sizeof(size_t)); free(a->strides); }
        if (a->data && a->data < hao_pool.base || a->data >= hao_pool.base + hao_pool.capacity) {
            account_free(a->bytes);
            free(a->data);
        }
    }
    account_free(sizeof(HaoArray));
    free(a);
}

void *hao_array_ptr(HaoArray *a, const size_t *indices) {
    if (!hao_array_check_bounds(a, indices)) return NULL;
    size_t offset = hao_offset(a, indices);
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
    if (new_count != a->element_count) return NULL;
    HaoArray *r = hao_array_create(a->type, rank, new_shape);
    if (!r) return NULL;
    if (a->element_count > 0) memcpy(r->data, a->data, a->bytes);
    return r;
}

static size_t compute_offset(const HaoArray *a, const size_t *idx) {
    return hao_offset(a, idx);
}

int hao_permute_axes(HaoArray *a, const size_t *perm, HaoArray **out) {
    if (!a || !perm || a->rank == 0) return -1;
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
    char *src = (char *)a->data;
    char *dst = (char *)(*out)->data;
    for (size_t lin = 0; lin < a->element_count; lin++) {
        size_t rem = lin;
        for (int i = (int)a->rank - 1; i >= 0; i--) {
            idx[i] = rem % a->shape[i];
            rem /= a->shape[i];
        }
        for (size_t i = 0; i < a->rank; i++) new_idx[perm[i]] = idx[i];
        size_t new_off = compute_offset(*out, new_idx);
        memcpy(dst + new_off * es, src + lin * es, es);
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
    char *src = (char *)a->data;
    char *dst = (char *)(*out)->data;
    for (size_t lin = 0; lin < a->element_count; lin++) {
        size_t rem = lin;
        for (int i = (int)a->rank - 1; i >= 0; i--) {
            idx[i] = rem % a->shape[i];
            rem /= a->shape[i];
        }
        idx[axis] = (idx[axis] + shift) % axis_len;
        size_t new_off = compute_offset(*out, idx);
        memcpy(dst + new_off * es, src + lin * es, es);
    }
    free(idx);
    return 0;
}

double hao_inner(const HaoArray *a, const HaoArray *b) {
    if (!a || !b || a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64 || a->element_count != b->element_count) return NAN;
    double s = 0.0;
    const double *ad = a->data, *bd = b->data;
    size_t n = a->element_count;
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        s += ad[i]*bd[i] + ad[i+1]*bd[i+1] + ad[i+2]*bd[i+2] + ad[i+3]*bd[i+3];
    }
    for (; i < n; i++) s += ad[i] * bd[i];
    return s;
}

HaoArray *hao_norm(const HaoArray *a) {
    if (!a || a->type != HAO_SCALAR_F64) return NULL;
    size_t sh[] = {1};
    HaoArray *n = hao_array_create(HAO_SCALAR_F64, 1, sh);
    if (!n) return NULL;
    double s = 0.0;
    const double *d = a->data;
    size_t m = a->element_count;
    size_t i = 0;
    for (; i + 3 < m; i += 4) {
        s += d[i]*d[i] + d[i+1]*d[i+1] + d[i+2]*d[i+2] + d[i+3]*d[i+3];
    }
    for (; i < m; i++) s += d[i] * d[i];
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
    size_t n = a->element_count;
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        od[i] = ad[i] / nv;
        od[i+1] = ad[i+1] / nv;
        od[i+2] = ad[i+2] / nv;
        od[i+3] = ad[i+3] / nv;
    }
    for (; i < n; i++) od[i] = ad[i] / nv;
    return out;
}

int hao_evolve_sphere(HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **out) {
    if (!x || !v || x->type != HAO_SCALAR_F64 || v->type != HAO_SCALAR_F64) return -1;
    if (x->element_count != v->element_count) return -2;
    double xn2 = 0.0, dot = 0.0;
    const double *xd0 = x->data, *vd0 = v->data;
    size_t n = x->element_count;
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        xn2 += xd0[i]*xd0[i] + xd0[i+1]*xd0[i+1] + xd0[i+2]*xd0[i+2] + xd0[i+3]*xd0[i+3];
        dot += xd0[i]*vd0[i] + xd0[i+1]*vd0[i+1] + xd0[i+2]*vd0[i+2] + xd0[i+3]*vd0[i+3];
    }
    for (; i < n; i++) {
        xn2 += xd0[i] * xd0[i];
        dot += xd0[i] * vd0[i];
    }
    if (fabs(xn2 - 1.0) > 1e-8 || fabs(dot) > 1e-8) return -3;
    *out = hao_array_create(x->type, x->rank, x->shape);
    if (!*out) return -1;
    memcpy((*out)->data, x->data, x->bytes);
    double *xd = (*out)->data;
    const double *vd = v->data;
    for (size_t s = 0; s < steps; s++) {
        double nv2 = 0.0;
        i = 0;
        for (; i + 3 < n; i += 4) nv2 += vd[i]*vd[i] + vd[i+1]*vd[i+1] + vd[i+2]*vd[i+2] + vd[i+3]*vd[i+3];
        for (; i < n; i++) nv2 += vd[i] * vd[i];
        double nv = sqrt(nv2) * fabs(eta);
        double c = cos(nv);
        double sincv = (fabs(nv) < 1e-12) ? 1.0 - nv*nv/6.0 : sin(nv)/nv;
        i = 0;
        for (; i + 3 < n; i += 4) {
            xd[i]   = c * xd[i]   + sincv * (vd[i]   * eta);
            xd[i+1] = c * xd[i+1] + sincv * (vd[i+1] * eta);
            xd[i+2] = c * xd[i+2] + sincv * (vd[i+2] * eta);
            xd[i+3] = c * xd[i+3] + sincv * (vd[i+3] * eta);
        }
        for (; i < n; i++) xd[i] = c * xd[i] + sincv * (vd[i] * eta);
    }
    return 0;
}

void hao_memory_report(void) {
    fprintf(stderr, "HAO memory: current=%zu peak=%zu allocs=%zu pool_used=%zu pool_peak=%zu\n",
            hao_total_bytes, hao_peak_bytes, hao_alloc_count, hao_pool.used, hao_pool.peak);
}
