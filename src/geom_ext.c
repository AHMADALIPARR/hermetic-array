/* HERMETIC ARRAY Ω (HAΩ)
 * Copyright (C) 2026 AHMAD ALI PARR
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* HERMETIC-Ω geometric extensions (additions only).
   Sphere log / distance / parallel transport / tangent projection,
   reflections, translations, coordinate change, determinants, SO(n)
   membership, full Cl(3,0) geometric algebra (product, outer product,
   left contraction, reversion, grade projection, rotors), a non-aliasing
   quaternion sandwich, and matrix group actions with orbit / stabilizer. */

#include "geom_ext.h"
#include "geom.h"
#include "array.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static double clamp_unit(double c) {
    if (c > 1.0) return 1.0;
    if (c < -1.0) return -1.0;
    return c;
}

double hao_sphere_distance(const double *x, const double *y, size_t n) {
    return acos(clamp_unit(hao_dot(x, y, n)));
}

/* Log_x(y): the tangent vector at x whose exponential reaches y. */
int hao_sphere_log(const double *x, const double *y, double *out, size_t n) {
    if (!x || !y || !out) return -1;
    double c = clamp_unit(hao_dot(x, y, n));
    double theta = acos(c);
    if (theta < 1e-12) {
        for (size_t i = 0; i < n; i++) out[i] = 0.0;
        return 0;
    }
    if (acos(-1.0) - theta < 1e-12) return -3;
    double un2 = 0.0;
    for (size_t i = 0; i < n; i++) {
        double u = y[i] - c * x[i];
        un2 += u * u;
    }
    double un = sqrt(un2);
    if (un == 0.0) return -3;
    double k = theta / un;
    for (size_t i = 0; i < n; i++) out[i] = k * (y[i] - c * x[i]);
    return 0;
}

/* Levi-Civita transport of v in T_x S^n to T_y S^n along the minimal geodesic. */
int hao_parallel_transport(const double *x, const double *y, const double *v, double *out, size_t n) {
    if (!x || !y || !v || !out) return -1;
    double denom = 1.0 + hao_dot(x, y, n);
    if (fabs(denom) < 1e-12) return -3;
    double k = hao_dot(y, v, n) / denom;
    for (size_t i = 0; i < n; i++) out[i] = v[i] - k * (x[i] + y[i]);
    return 0;
}

void hao_project_tangent(const double *x, const double *v, double *out, size_t n) {
    double d = hao_dot(x, v, n);
    for (size_t i = 0; i < n; i++) out[i] = v[i] - d * x[i];
}

double hao_sphere_sectional_curvature(double radius) {
    if (!(radius > 0.0)) return NAN;
    return 1.0 / (radius * radius);
}

/* Householder reflection across the hyperplane orthogonal to normal. */
int hao_reflect(const double *x, const double *normal, double *out, size_t n) {
    if (!x || !normal || !out) return -1;
    double nn = hao_norm2(normal, n);
    if (nn == 0.0) return -3;
    double k = 2.0 * hao_dot(x, normal, n) / nn;
    for (size_t i = 0; i < n; i++) out[i] = x[i] - k * normal[i];
    return 0;
}

void hao_translate(const double *x, const double *t, double *out, size_t n) {
    hao_vec4_add_mul(out, x, t, 1.0, n);
}

/* Coordinates of x in the orthonormal basis whose rows are B. */
int hao_change_basis(const double *B, const double *x, double *out, size_t n) {
    if (!B || !x || !out) return -1;
    if (!hao_is_orthogonal(B, n, 1e-9)) return -3;
    for (size_t i = 0; i < n; i++) out[i] = hao_dot(B + i * n, x, n);
    return 0;
}

double hao_det(const double *M, size_t n) {
    if (!M) return NAN;
    if (n == 0) return 1.0;
    double *a = malloc(n * n * sizeof(double));
    if (!a) return NAN;
    memcpy(a, M, n * n * sizeof(double));
    double det = 1.0;
    for (size_t c = 0; c < n; c++) {
        size_t p = c;
        for (size_t r = c + 1; r < n; r++) if (fabs(a[r * n + c]) > fabs(a[p * n + c])) p = r;
        if (a[p * n + c] == 0.0) { free(a); return 0.0; }
        if (p != c) {
            for (size_t k = 0; k < n; k++) {
                double t = a[c * n + k]; a[c * n + k] = a[p * n + k]; a[p * n + k] = t;
            }
            det = -det;
        }
        double piv = a[c * n + c];
        det *= piv;
        for (size_t r = c + 1; r < n; r++) {
            double f = a[r * n + c] / piv;
            for (size_t k = c; k < n; k++) a[r * n + k] -= f * a[c * n + k];
        }
    }
    free(a);
    return det;
}

bool hao_is_special_orthogonal(const double *Q, size_t n, double tol) {
    return hao_is_orthogonal(Q, n, tol) && fabs(hao_det(Q, n) - 1.0) < tol;
}

void hao_wedge3(const double a[3], const double b[3], double out[3]) {
    double e23 = a[1]*b[2] - a[2]*b[1];
    double e31 = a[2]*b[0] - a[0]*b[2];
    double e12 = a[0]*b[1] - a[1]*b[0];
    out[0] = e23; out[1] = e31; out[2] = e12;
}

/* ab = a·b + a∧b for 3-vectors: [scalar, e23, e31, e12]. */
int hao_geometric_product3(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (!a || !b || !out) return -1;
    if (a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64) return -1;
    if (a->element_count != 3 || b->element_count != 3) return -2;
    size_t sh[1] = {4};
    *out = hao_array_create(HAO_SCALAR_F64, 1, sh);
    if (!*out) return -1;
    const double *ad = a->data, *bd = b->data;
    double *od = (*out)->data;
    od[0] = hao_dot(ad, bd, 3);
    hao_wedge3(ad, bd, od + 1);
    return 0;
}

static int popcount3(unsigned m) {
    return (int)((m & 1u) + ((m >> 1) & 1u) + ((m >> 2) & 1u));
}

/* Sign of the reordering e_a e_b -> e_(a xor b) in a Euclidean metric. */
static double blade_sign(unsigned a, unsigned b) {
    int swaps = 0;
    a >>= 1;
    while (a) {
        swaps += popcount3(a & b);
        a >>= 1;
    }
    return (swaps & 1) ? -1.0 : 1.0;
}

typedef enum { MV_GEOMETRIC, MV_OUTER, MV_LEFT } MvKind;

static void mv_mul(const double *a, const double *b, double *out, MvKind kind) {
    double r[HAO_MV_DIM] = {0};
    for (unsigned i = 0; i < HAO_MV_DIM; i++) {
        if (a[i] == 0.0) continue;
        for (unsigned j = 0; j < HAO_MV_DIM; j++) {
            if (b[j] == 0.0) continue;
            if (kind == MV_OUTER && (i & j)) continue;
            if (kind == MV_LEFT && (i & j) != i) continue;
            r[i ^ j] += blade_sign(i, j) * a[i] * b[j];
        }
    }
    memcpy(out, r, sizeof(r));
}

void hao_mv_product(const double a[HAO_MV_DIM], const double b[HAO_MV_DIM], double out[HAO_MV_DIM]) {
    mv_mul(a, b, out, MV_GEOMETRIC);
}

void hao_mv_outer(const double a[HAO_MV_DIM], const double b[HAO_MV_DIM], double out[HAO_MV_DIM]) {
    mv_mul(a, b, out, MV_OUTER);
}

void hao_mv_left_contract(const double a[HAO_MV_DIM], const double b[HAO_MV_DIM], double out[HAO_MV_DIM]) {
    mv_mul(a, b, out, MV_LEFT);
}

void hao_mv_reverse(const double a[HAO_MV_DIM], double out[HAO_MV_DIM]) {
    for (unsigned i = 0; i < HAO_MV_DIM; i++) {
        int k = popcount3(i);
        out[i] = ((k * (k - 1) / 2) & 1) ? -a[i] : a[i];
    }
}

int hao_mv_grade(const double a[HAO_MV_DIM], int k, double out[HAO_MV_DIM]) {
    if (k < 0 || k > 3) return -3;
    for (unsigned i = 0; i < HAO_MV_DIM; i++) out[i] = (popcount3(i) == k) ? a[i] : 0.0;
    return 0;
}

void hao_mv_from_vector(const double v[3], double out[HAO_MV_DIM]) {
    for (unsigned i = 0; i < HAO_MV_DIM; i++) out[i] = 0.0;
    out[1] = v[0]; out[2] = v[1]; out[4] = v[2];
}

/* R = cos(θ/2) - sin(θ/2) B, B the unit bivector dual to the axis;
   R v R~ rotates v by θ right-handedly about the axis. */
void hao_rotor_from_axis_angle(const double axis[3], double angle, double out[HAO_MV_DIM]) {
    for (unsigned i = 0; i < HAO_MV_DIM; i++) out[i] = 0.0;
    double nn = sqrt(hao_norm2(axis, 3));
    out[0] = 1.0;
    if (nn == 0.0) return;
    double c = cos(angle / 2.0), s = sin(angle / 2.0);
    double n1 = axis[0] / nn, n2 = axis[1] / nn, n3 = axis[2] / nn;
    out[0] = c;
    out[6] = -s * n1;   /* e23 */
    out[5] =  s * n2;   /* e13 = -e31 */
    out[3] = -s * n3;   /* e12 */
}

void hao_rotor_apply(const double rotor[HAO_MV_DIM], const double v[3], double out[3]) {
    double mv[HAO_MV_DIM], rr[HAO_MV_DIM], t[HAO_MV_DIM], r[HAO_MV_DIM];
    hao_mv_from_vector(v, mv);
    hao_mv_reverse(rotor, rr);
    hao_mv_product(rotor, mv, t);
    hao_mv_product(t, rr, r);
    out[0] = r[1]; out[1] = r[2]; out[2] = r[4];
}

int hao_quat_sandwich(const double q[4], const double v[3], double out[3]) {
    if (!q || !v || !out) return -1;
    double n2 = q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3];
    if (n2 == 0.0) return -3;
    double qv[4] = {0.0, v[0], v[1], v[2]};
    double qc[4] = {q[0], -q[1], -q[2], -q[3]};
    double t1[4], t2[4];
    hao_quat_mul(q, qv, t1);
    hao_quat_mul(t1, qc, t2);
    out[0] = t2[1] / n2; out[1] = t2[2] / n2; out[2] = t2[3] / n2;
    return 0;
}

/* rotor: 4 coefficients (quaternion w,x,y,z) or 8 (Cl(3,0) multivector). */
int hao_rotor_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out) {
    if (!rotor || !vec || !out) return -1;
    if (rotor->type != HAO_SCALAR_F64 || vec->type != HAO_SCALAR_F64) return -1;
    if (vec->element_count != 3) return -2;
    if (rotor->element_count != 4 && rotor->element_count != HAO_MV_DIM) return -2;
    *out = hao_array_create(HAO_SCALAR_F64, vec->rank, vec->shape);
    if (!*out) return -1;
    const double *rd = rotor->data, *vd = vec->data;
    double *od = (*out)->data;
    if (rotor->element_count == 4) {
        int rc = hao_quat_sandwich(rd, vd, od);
        if (rc) { hao_array_free(*out); *out = NULL; }
        return rc;
    }
    hao_rotor_apply(rd, vd, od);
    return 0;
}

/* g: n×n matrix, x: any array whose last axis has length n. */
int hao_group_act_matrix(const HaoArray *g, const HaoArray *x, HaoArray **out) {
    if (!g || !x || !out) return -1;
    if (g->type != HAO_SCALAR_F64 || x->type != HAO_SCALAR_F64) return -1;
    if (g->rank != 2 || g->shape[0] != g->shape[1] || x->rank < 1) return -4;
    size_t n = g->shape[0];
    if (x->shape[x->rank - 1] != n) return -2;
    *out = hao_array_create(HAO_SCALAR_F64, x->rank, x->shape);
    if (!*out) return -1;
    const double *gd = g->data, *xd = x->data;
    double *od = (*out)->data;
    size_t nvec = (n == 0) ? 0 : x->element_count / n;
    for (size_t v = 0; v < nvec; v++) {
        for (size_t i = 0; i < n; i++) od[v * n + i] = hao_dot(gd + i * n, xd + v * n, n);
    }
    return 0;
}

/* Rows x, g x, g² x, …, g^steps x. */
int hao_orbit(const HaoArray *g, const HaoArray *x, size_t steps, HaoArray **out) {
    if (!g || !x || !out) return -1;
    if (x->rank != 1) return -4;
    size_t n = x->shape[0];
    size_t sh[2] = {steps + 1, n};
    *out = hao_array_create(HAO_SCALAR_F64, 2, sh);
    if (!*out) return -1;
    double *od = (*out)->data;
    memcpy(od, x->data, n * sizeof(double));
    HaoArray *cur = NULL;
    size_t vs[1] = {n};
    cur = hao_array_create(HAO_SCALAR_F64, 1, vs);
    if (!cur) { hao_array_free(*out); *out = NULL; return -1; }
    memcpy(cur->data, x->data, n * sizeof(double));
    for (size_t s = 1; s <= steps; s++) {
        HaoArray *nxt = NULL;
        int rc = hao_group_act_matrix(g, cur, &nxt);
        hao_array_free(cur);
        if (rc) { hao_array_free(*out); *out = NULL; return rc; }
        memcpy(od + s * n, nxt->data, n * sizeof(double));
        cur = nxt;
    }
    hao_array_free(cur);
    return 0;
}

bool hao_stabilizes(const HaoArray *g, const HaoArray *x, double tol) {
    HaoArray *gx = NULL;
    if (hao_group_act_matrix(g, x, &gx) != 0) return false;
    bool ok = true;
    const double *a = gx->data, *b = x->data;
    for (size_t i = 0; i < x->element_count; i++) if (fabs(a[i] - b[i]) > tol) { ok = false; break; }
    hao_array_free(gx);
    return ok;
}
