#include "geom.h"
#include "array.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double hao_norm2(const double *v, size_t n) {
    double s = 0.0;
    for (size_t i = 0; i < n; i++) s += v[i] * v[i];
    return s;
}

double hao_dot(const double *a, const double *b, size_t n) {
    double s = 0.0;
    for (size_t i = 0; i < n; i++) s += a[i] * b[i];
    return s;
}

static double hao_sinc(double t) {
    if (fabs(t) < 1e-12) return 1.0 - t * t / 6.0;
    return sin(t) / t;
}

void hao_sphere_exp(const double *x, const double *v, double *out, size_t n) {
    double nv2 = hao_norm2(v, n);
    double nv = sqrt(nv2);
    double c = cos(nv);
    double s = hao_sinc(nv);
    for (size_t i = 0; i < n; i++) out[i] = c * x[i] + s * v[i];
}

bool hao_is_unit(const double *x, size_t n, double tol) {
    return fabs(hao_norm2(x, n) - 1.0) < tol;
}

bool hao_is_tangent(const double *x, const double *v, size_t n, double tol) {
    return fabs(hao_dot(x, v, n)) < tol;
}

int hao_array_sphere_exp(HaoArray *x, HaoArray *v, HaoArray *out) {
    if (!x || !v || !out) return -1;
    if (x->type != HAO_SCALAR_F64 || v->type != HAO_SCALAR_F64 || out->type != HAO_SCALAR_F64) return -1;
    if (x->element_count != v->element_count || x->element_count != out->element_count) return -2;
    size_t vec_dim = x->shape[x->rank - 1];
    size_t nvecs = x->element_count / vec_dim;
    double *xd = (double *)x->data;
    double *vd = (double *)v->data;
    double *od = (double *)out->data;
    for (size_t i = 0; i < nvecs; i++) {
        if (!hao_is_unit(xd + i * vec_dim, vec_dim, 1e-8)) return -3;
        if (!hao_is_tangent(xd + i * vec_dim, vd + i * vec_dim, vec_dim, 1e-8)) return -4;
        hao_sphere_exp(xd + i * vec_dim, vd + i * vec_dim, od + i * vec_dim, vec_dim);
    }
    return 0;
}

int hao_geometric_product(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64) return -1;
    if (a->element_count != b->element_count) return -2;
    *out = hao_array_create(HAO_SCALAR_F64, a->rank, a->shape);
    if (!*out) return -1;
    double *ad = a->data, *bd = b->data, *od = (*out)->data;
    size_t n = a->element_count;
    double s = 0.0;
    for (size_t i = 0; i < n; i++) s += ad[i] * bd[i];
    od[0] = s;
    if (n >= 3) {
        od[1] = ad[1]*bd[2] - ad[2]*bd[1];
        od[2] = ad[2]*bd[0] - ad[0]*bd[2];
        if (n > 3) od[3] = ad[0]*bd[1] - ad[1]*bd[0];
    }
    return 0;
}

int hao_exterior(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    return hao_geometric_product(a, b, out);
}

int hao_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out) {
    if (!rotor || !vec || vec->element_count != 3) return -1;
    *out = hao_array_create(HAO_SCALAR_F64, vec->rank, vec->shape);
    if (!*out) return -1;
    memcpy((*out)->data, vec->data, 3 * sizeof(double));
    return 0;
}

int hao_group_act(const HaoArray *g, const HaoArray *x, HaoArray **out) {
    *out = hao_array_create(x->type, x->rank, x->shape);
    if (!*out) return -1;
    memcpy((*out)->data, x->data, x->element_count * hao_array_element_size(x->type));
    return 0;
}
