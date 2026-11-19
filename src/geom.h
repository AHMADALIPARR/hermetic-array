#ifndef HAO_GEOM_H
#define HAO_GEOM_H

#include "array.h"

/* Geometric product for multivectors (simplified 3D) */
int hao_geometric_product(const HaoArray *a, const HaoArray *b, HaoArray **out);

/* Exterior product */
int hao_exterior(const HaoArray *a, const HaoArray *b, HaoArray **out);

/* Rotor sandwich */
int hao_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out);

/* Group action on points */
int hao_group_act(const HaoArray *g, const HaoArray *x, HaoArray **out);

#endif
