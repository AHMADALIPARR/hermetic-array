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

#ifndef HAO_GEOM_H
#define HAO_GEOM_H

#include "array.h"
#include <stdbool.h>

double hao_norm2(const double *v, size_t n);
double hao_dot(const double *a, const double *b, size_t n);
void hao_sphere_exp(const double *x, const double *v, double *out, size_t n);
bool hao_is_unit(const double *x, size_t n, double tol);
bool hao_is_tangent(const double *x, const double *v, size_t n, double tol);
bool hao_is_orthogonal(const double *Q, size_t n, double tol);
bool hao_is_rotation(const double *Q, size_t n, double tol);
int hao_array_sphere_exp(HaoArray *x, HaoArray *v, HaoArray *out);
int hao_geometric_product(const HaoArray *a, const HaoArray *b, HaoArray **out);
int hao_exterior(const HaoArray *a, const HaoArray *b, HaoArray **out);
int hao_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out);
int hao_group_act(const HaoArray *g, const HaoArray *x, HaoArray **out);
int hao_geodesic(const HaoArray *x, const HaoArray *y, double t, HaoArray **out);
int hao_quat_mul(const double q1[4], const double q2[4], double out[4]);
int hao_quat_rotate(const double q[4], const double v[3], double out[3]);

#endif
