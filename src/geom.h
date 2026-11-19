#ifndef HAO_GEOM_H
#define HAO_GEOM_H

#include "array.h"
#include <math.h>

/* Unit sphere exponential map: Exp_x(v) = cos(||v||) x + sinc(||v||) v
   Requires ||x|| == 1 and v tangent (x·v == 0). Continuous at 0. */
double hao_norm2(const double *v, size_t n);
double hao_dot(const double *a, const double *b, size_t n);
void hao_sphere_exp(const double *x, const double *v, double *out, size_t n);
bool hao_is_unit(const double *x, size_t n, double tol);
bool hao_is_tangent(const double *x, const double *v, size_t n, double tol);

/* Array versions operate on last axis as vector dimension */
int hao_array_sphere_exp(HaoArray *x, HaoArray *v, HaoArray *out);

#endif
