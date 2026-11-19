#include "geom.h"
#include <math.h>
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
    if (fabs(t) < 1e-12) return 1.0 - t*t/6.0; /* Taylor */
    return sin(t) / t;
}

void hao_sphere_exp(const double *x, const double *v, double *out, size_t n) {
    double nv2 = hao_norm2(v, n);
    double nv = sqrt(nv2);
    double c = cos(nv);
    double s = hao_sinc(nv);
    for (size_t i = 0; i < n; i++) {
        out[i] = c * x[i] + s * v[i];
    }
}

bool hao_is_unit(const double *x, size_t n, double tol) {
    return fabs(hao_norm2(x, n) - 1.0) < tol;
}

bool hao_is_tangent(const double *x, const double *v, size_t n, double tol) {
    return fabs(hao_dot(x, v, n)) < tol;
}

int hao_array_sphere_exp(HaoArray *x, HaoArray *v, HaoArray *out) {
    if (x->type != HAO_SCALAR_F64 || v->type != HAO_SCALAR_F64 || out->type != HAO_SCALAR_F64) return -1;
    if (x->rank != v->rank || x->rank != out->rank) return -2;
    for (size_t i = 0; i < x->rank; i++) {
        if (x->shape[i] != v->shape[i] || x->shape[i] != out->shape[i]) return -3;
    }
    size_t vec_dim = x->shape[x->rank - 1];
    size_t nvecs = x->element_count / vec_dim;
    double *xd = (double *)x->data;
    double *vd = (double *)v->data;
    double *od = (double *)out->data;
    for (size_t i = 0; i < nvecs; i++) {
        hao_sphere_exp(xd + i*vec_dim, vd + i*vec_dim, od + i*vec_dim, vec_dim);
    }
    return 0;
}
