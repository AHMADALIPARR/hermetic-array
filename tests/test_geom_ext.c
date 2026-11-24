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

#include "../src/array.h"
#include "../src/geom.h"
#include "../src/array_ext.h"
#include "../src/geom_ext.h"
#include <stdio.h>
#include <math.h>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static bool near(double a, double b, double tol) { return fabs(a - b) < tol; }
static bool near3(const double *a, double x, double y, double z) {
    return near(a[0], x, 1e-12) && near(a[1], y, 1e-12) && near(a[2], z, 1e-12);
}

int main(void) {
    const double PI = acos(-1.0);
    double x[3] = {1, 0, 0}, y[3] = {0, 1, 0}, z[3] = {0, 0, 1};
    double out[3], back[3];

    /* sphere metric: distance, log is the inverse of exp, transport */
    CHECK(near(hao_sphere_distance(x, y, 3), PI / 2, 1e-12));
    CHECK(hao_sphere_log(x, y, out, 3) == 0 && near3(out, 0, PI / 2, 0));
    hao_sphere_exp(x, out, back, 3);
    CHECK(near3(back, 0, 1, 0));
    double p[3] = {0.6, 0.8, 0};
    CHECK(hao_sphere_log(x, p, out, 3) == 0);
    hao_sphere_exp(x, out, back, 3);
    CHECK(near3(back, 0.6, 0.8, 0));
    double mx[3] = {-1, 0, 0};
    CHECK(hao_sphere_log(x, mx, out, 3) == -3);
    CHECK(hao_sphere_log(x, x, out, 3) == 0 && near3(out, 0, 0, 0));

    double v[3] = {0, 0.3, 0.4};               /* tangent at x */
    CHECK(hao_parallel_transport(x, y, v, out, 3) == 0);
    CHECK(hao_is_tangent(y, out, 3, 1e-12));
    CHECK(near(sqrt(hao_norm2(out, 3)), 0.5, 1e-12));
    CHECK(near3(out, -0.3, 0, 0.4));
    CHECK(hao_parallel_transport(x, mx, v, out, 3) == -3);

    double w[3] = {0.5, 0.5, 0.5};
    hao_project_tangent(x, w, out, 3);
    CHECK(near3(out, 0, 0.5, 0.5) && hao_is_tangent(x, out, 3, 1e-12));
    CHECK(near(hao_sphere_sectional_curvature(2.0), 0.25, 1e-15));
    double k0 = hao_sphere_sectional_curvature(0.0);
    CHECK(k0 != k0);   /* NaN; isnan() needs long double under CompCert */

    /* Euclidean transforms */
    double n[3] = {0, 2, 0}, q[3] = {1, 2, 3};
    CHECK(hao_reflect(q, n, out, 3) == 0 && near3(out, 1, -2, 3));
    double zero[3] = {0, 0, 0};
    CHECK(hao_reflect(q, zero, out, 3) == -3);
    hao_translate(q, q, out, 3);
    CHECK(near3(out, 2, 4, 6));
    double B[9] = {0, 1, 0,  0, 0, 1,  1, 0, 0};
    CHECK(hao_change_basis(B, q, out, 3) == 0 && near3(out, 2, 3, 1));
    double notB[9] = {1, 1, 0,  0, 1, 0,  0, 0, 1};
    CHECK(hao_change_basis(notB, q, out, 3) == -3);

    double I3[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    double Rz[9] = {0, -1, 0,  1, 0, 0,  0, 0, 1};
    double F[9] = {1, 0, 0,  0, 1, 0,  0, 0, -1};
    double M[9] = {2, 0, 1,  1, 3, 2,  1, 1, 2};
    CHECK(near(hao_det(I3, 3), 1, 1e-12) && near(hao_det(M, 3), 6, 1e-12) && near(hao_det(F, 3), -1, 1e-12));
    CHECK(hao_is_special_orthogonal(Rz, 3, 1e-12));
    CHECK(hao_is_orthogonal(F, 3, 1e-12) && !hao_is_special_orthogonal(F, 3, 1e-12));

    /* GA: wedge is the bivector dual of the cross product, ab = a·b + a∧b */
    hao_wedge3(x, y, out);
    CHECK(near3(out, 0, 0, 1));
    hao_wedge3(y, z, out);
    CHECK(near3(out, 1, 0, 0));
    double a1[3] = {1, 2, 3}, b1[3] = {4, 5, 6};
    hao_wedge3(a1, b1, out);
    CHECK(near3(out, -3, 6, -3));
    size_t s3[1] = {3};
    HaoArray *A = NULL, *Bv = NULL, *G = NULL;
    hao_from_f64(a1, 1, s3, &A);
    hao_from_f64(b1, 1, s3, &Bv);
    CHECK(hao_geometric_product3(A, Bv, &G) == 0 && G->element_count == 4);
    CHECK(near(hao_get_f64(G, 0), 32, 1e-12) && near(hao_get_f64(G, 1), -3, 1e-12) &&
          near(hao_get_f64(G, 2), 6, 1e-12) && near(hao_get_f64(G, 3), -3, 1e-12));
    hao_array_free(G);

    /* Cl(3,0) multivector algebra (bitmask basis 1 e1 e2 e12 e3 e13 e23 e123) */
    double e1[8] = {0, 1, 0, 0, 0, 0, 0, 0}, e2[8] = {0, 0, 1, 0, 0, 0, 0, 0}, e3[8] = {0, 0, 0, 0, 1, 0, 0, 0};
    double r[8], t[8];
    hao_mv_product(e1, e1, r);
    CHECK(near(r[0], 1, 1e-15));
    hao_mv_product(e1, e2, r);
    CHECK(near(r[3], 1, 1e-15) && near(r[0], 0, 1e-15));
    hao_mv_product(e2, e1, t);
    CHECK(near(t[3], -1, 1e-15));
    hao_mv_product(r, r, t);                        /* e12 e12 = -1 */
    CHECK(near(t[0], -1, 1e-15));
    hao_mv_product(r, e3, t);                       /* e12 e3 = e123 */
    CHECK(near(t[7], 1, 1e-15));
    hao_mv_product(t, t, r);                        /* I² = -1 */
    CHECK(near(r[0], -1, 1e-15));
    hao_mv_outer(e1, e1, r);
    CHECK(near(r[0], 0, 1e-15));
    double e12[8] = {0, 0, 0, 1, 0, 0, 0, 0};
    hao_mv_left_contract(e1, e12, r);               /* e1 ⌋ e12 = e2 */
    CHECK(near(r[2], 1, 1e-15) && near(r[3], 0, 1e-15));
    hao_mv_left_contract(e12, e1, r);               /* grade too high: 0 */
    CHECK(near(r[1], 0, 1e-15) && near(r[2], 0, 1e-15));
    double mv[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    hao_mv_reverse(mv, r);
    CHECK(r[0] == 1 && r[1] == 2 && r[3] == -4 && r[5] == -6 && r[6] == -7 && r[7] == -8);
    CHECK(hao_mv_grade(mv, 2, r) == 0 && r[3] == 4 && r[5] == 6 && r[6] == 7 && r[0] == 0 && r[7] == 0);
    CHECK(hao_mv_grade(mv, 4, r) == -3);

    /* rotors: 90° about z sends e1 to e2; agrees with the quaternion sandwich */
    double R[8], vr[3], vq[3];
    hao_rotor_from_axis_angle(z, PI / 2, R);
    hao_rotor_apply(R, x, vr);
    CHECK(near3(vr, 0, 1, 0));
    double axis[3] = {1, 1, 1}, u[3] = {0.3, -1.2, 2.0};
    hao_rotor_from_axis_angle(axis, 0.7, R);
    hao_rotor_apply(R, u, vr);
    double s = sin(0.35) / sqrt(3.0);
    double quat[4] = {cos(0.35), s, s, s};
    CHECK(hao_quat_sandwich(quat, u, vq) == 0);
    CHECK(near(vr[0], vq[0], 1e-12) && near(vr[1], vq[1], 1e-12) && near(vr[2], vq[2], 1e-12));
    CHECK(near(hao_norm2(vr, 3), hao_norm2(u, 3), 1e-12));
    double q0[4] = {0, 0, 0, 0};
    CHECK(hao_quat_sandwich(q0, u, vq) == -3);

    size_t s4[1] = {4}, s8[1] = {8};
    HaoArray *Q = NULL, *Rv = NULL, *X = NULL, *Out = NULL;
    double qz[4] = {cos(PI / 4), 0, 0, sin(PI / 4)};
    hao_from_f64(qz, 1, s4, &Q);
    hao_from_f64(x, 1, s3, &X);
    CHECK(hao_rotor_sandwich(Q, X, &Out) == 0 && near3(Out->data, 0, 1, 0));
    hao_array_free(Out);
    hao_rotor_from_axis_angle(z, PI / 2, R);
    hao_from_f64(R, 1, s8, &Rv);
    CHECK(hao_rotor_sandwich(Rv, X, &Out) == 0 && near3(Out->data, 0, 1, 0));
    hao_array_free(Out);

    /* group action of SO(3) on vectors, orbit, stabilizer */
    size_t s33[2] = {3, 3};
    HaoArray *Gz = NULL, *Gm = NULL, *Zv = NULL;
    hao_from_f64(Rz, 2, s33, &Gz);
    CHECK(hao_group_act_matrix(Gz, X, &Out) == 0 && near3(Out->data, 0, 1, 0));
    hao_array_free(Out);
    double batch[6] = {1, 0, 0, 0, 1, 0};
    size_t s23[2] = {2, 3};
    HaoArray *Xb = NULL;
    hao_from_f64(batch, 2, s23, &Xb);
    CHECK(hao_group_act_matrix(Gz, Xb, &Out) == 0 && near3(Out->data, 0, 1, 0) && near3((double *)Out->data + 3, -1, 0, 0));
    hao_array_free(Out);
    CHECK(hao_orbit(Gz, X, 4, &Out) == 0 && Out->shape[0] == 5 && Out->shape[1] == 3);
    CHECK(near3((double *)Out->data + 6, -1, 0, 0) && near3((double *)Out->data + 12, 1, 0, 0));
    hao_array_free(Out);
    hao_from_f64(z, 1, s3, &Zv);
    CHECK(hao_stabilizes(Gz, Zv, 1e-12) && !hao_stabilizes(Gz, X, 1e-12));
    hao_from_f64(M, 2, s33, &Gm);
    CHECK(hao_group_act_matrix(Gm, A, &Out) == 0 && near3(Out->data, 5, 13, 9));
    hao_array_free(Out);

    hao_array_free(A); hao_array_free(Bv); hao_array_free(Q); hao_array_free(Rv); hao_array_free(X);
    hao_array_free(Gz); hao_array_free(Gm); hao_array_free(Zv); hao_array_free(Xb);
    if (failures) { printf("geom ext tests: %d failure(s)\n", failures); return 1; }
    printf("geom ext tests passed\n");
    return 0;
}
