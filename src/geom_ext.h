/*
 * HERMETIC-Ω (hermetic-array) · src/geom_ext.h
 * SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
 * License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
 * Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
 * Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
 * Hark, though this node be but a spark,
 * Its covenant endureth through the dark.
 */

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
   Completes the metric-geometry, geometric-algebra and group arrays
   listed in docs/operators/README.md and lang/ga.bqn, lang/group.bqn,
   lang/types.bqn that had no full C11 kernel. The original kernels in
   geom.c (including hao_sandwich and hao_group_act) are left untouched. */

#ifndef HAO_GEOM_EXT_H
#define HAO_GEOM_EXT_H

#include "array.h"
#include <stdbool.h>

/* Cl(3,0) multivectors: 8 coefficients indexed by blade bitmask
   0:1  1:e1  2:e2  3:e12  4:e3  5:e13  6:e23  7:e123 */
#define HAO_MV_DIM 8

/* Metric geometry on the unit sphere */
double hao_sphere_distance(const double *x, const double *y, size_t n);
int hao_sphere_log(const double *x, const double *y, double *out, size_t n);
int hao_parallel_transport(const double *x, const double *y, const double *v, double *out, size_t n);
void hao_project_tangent(const double *x, const double *v, double *out, size_t n);
double hao_sphere_sectional_curvature(double radius);

/* Euclidean transforms */
int hao_reflect(const double *x, const double *normal, double *out, size_t n);
void hao_translate(const double *x, const double *t, double *out, size_t n);
int hao_change_basis(const double *B, const double *x, double *out, size_t n);
double hao_det(const double *M, size_t n);
bool hao_is_special_orthogonal(const double *Q, size_t n, double tol);

/* Geometric algebra, 3D */
void hao_wedge3(const double a[3], const double b[3], double out[3]);
int hao_geometric_product3(const HaoArray *a, const HaoArray *b, HaoArray **out);
void hao_mv_product(const double a[HAO_MV_DIM], const double b[HAO_MV_DIM], double out[HAO_MV_DIM]);
void hao_mv_outer(const double a[HAO_MV_DIM], const double b[HAO_MV_DIM], double out[HAO_MV_DIM]);
void hao_mv_left_contract(const double a[HAO_MV_DIM], const double b[HAO_MV_DIM], double out[HAO_MV_DIM]);
void hao_mv_reverse(const double a[HAO_MV_DIM], double out[HAO_MV_DIM]);
int hao_mv_grade(const double a[HAO_MV_DIM], int k, double out[HAO_MV_DIM]);
void hao_mv_from_vector(const double v[3], double out[HAO_MV_DIM]);
void hao_rotor_from_axis_angle(const double axis[3], double angle, double out[HAO_MV_DIM]);
void hao_rotor_apply(const double rotor[HAO_MV_DIM], const double v[3], double out[3]);

/* Quaternion sandwich q v q* without aliasing; q need not be unit. */
int hao_quat_sandwich(const double q[4], const double v[3], double out[3]);
int hao_rotor_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out);

/* Group actions of GL(n) / SO(n) on batches of vectors */
int hao_group_act_matrix(const HaoArray *g, const HaoArray *x, HaoArray **out);
int hao_orbit(const HaoArray *g, const HaoArray *x, size_t steps, HaoArray **out);
bool hao_stabilizes(const HaoArray *g, const HaoArray *x, double tol);

#endif
