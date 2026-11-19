#include "geom.h"
#include "array.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double hao_norm2(const double *v, size_t n) {
    double s = 0.0;
    const double *end = v + n;
    for (; v < end; v++) s += (*v) * (*v);
    return s;
}

double hao_dot(const double *a, const double *b, size_t n) {
    double s = 0.0;
    const double *ae = a + n;
    for (; a < ae; a++, b++) s += (*a) * (*b);
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
    const double *xe = x + n;
    for (; x < xe; x++, v++, out++) *out = c * (*x) + s * (*v);
}

bool hao_is_unit(const double *x, size_t n, double tol) {
    return fabs(hao_norm2(x, n) - 1.0) < tol;
}

bool hao_is_tangent(const double *x, const double *v, size_t n, double tol) {
    return fabs(hao_dot(x, v, n)) < tol;
}

bool hao_is_orthogonal(const double *Q, size_t n, double tol) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            double d = 0.0;
            const double *qi = Q + i * n;
            const double *qj = Q + j * n;
            for (size_t k = 0; k < n; k++) d += qi[k] * qj[k];
            double expected = (i == j) ? 1.0 : 0.0;
            if (fabs(d - expected) > tol) return false;
        }
    }
    return true;
}

bool hao_is_rotation(const double *Q, size_t n, double tol) {
    return hao_is_orthogonal(Q, n, tol);
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
        double *xptr = xd + i * vec_dim;
        double *vptr = vd + i * vec_dim;
        double *optr = od + i * vec_dim;
        if (!hao_is_unit(xptr, vec_dim, 1e-8)) return -3;
        if (!hao_is_tangent(xptr, vptr, vec_dim, 1e-8)) return -4;
        hao_sphere_exp(xptr, vptr, optr, vec_dim);
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
    const double *ae = ad + n;
    for (; ad < ae; ad++, bd++) s += (*ad) * (*bd);
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
    memcpy((*out)->data, x->data, x->bytes);
    return 0;
}

int hao_geodesic(const HaoArray *x, const HaoArray *y, double t, HaoArray **out) {
    if (!x || !y || x->element_count != y->element_count) return -1;
    size_t n = x->element_count;
    double *xd = x->data, *yd = y->data;
    double c = hao_dot(xd, yd, n);
    if (c > 1.0) c = 1.0;
    if (c < -1.0) c = -1.0;
    double theta = acos(c);
    *out = hao_array_create(HAO_SCALAR_F64, x->rank, x->shape);
    if (!*out) return -1;
    double *od = (*out)->data;
    if (fabs(theta) < 1e-12) {
        memcpy(od, xd, n * sizeof(double));
        return 0;
    }
    double s = sin(theta);
    double a = sin((1.0 - t) * theta) / s;
    double b = sin(t * theta) / s;
    const double *xe = xd + n;
    for (; xd < xe; xd++, yd++, od++) *od = a * (*xd) + b * (*yd);
    return 0;
}

int hao_quat_mul(const double q1[4], const double q2[4], double out[4]) {
    out[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    out[1] = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2];
    out[2] = q1[0]*q2[2] - q1[1]*q2[3] + q1[2]*q2[0] + q1[3]*q2[1];
    out[3] = q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0];
    return 0;
}

int hao_quat_rotate(const double q[4], const double v[3], double out[3]) {
    double qv[4] = {0.0, v[0], v[1], v[2]};
    double qinv[4] = {q[0], -q[1], -q[2], -q[3]};
    double tmp[4];
    hao_quat_mul(q, qv, tmp);
    hao_quat_mul(tmp, qinv, tmp);
    out[0] = tmp[1]; out[1] = tmp[2]; out[2] = tmp[3];
    return 0;
}
