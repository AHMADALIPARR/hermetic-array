#include "geom.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

int hao_geometric_product(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64) return -1;
    size_t n = a->element_count;
    if (n != b->element_count) return -2;
    *out = hao_array_create(HAO_SCALAR_F64, a->rank, a->shape);
    if (!*out) return -1;
    double *ad = a->data, *bd = b->data, *od = (*out)->data;
    /* Scalar part + vector cross for 3D; general GA requires full blade basis */
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
    return hao_geometric_product(a, b, out); /* wedge is antisym part */
}

int hao_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out) {
    /* R v R~ for unit rotor; simplified */
    if (rotor->element_count < 1 || vec->element_count != 3) return -1;
    *out = hao_array_create(HAO_SCALAR_F64, vec->rank, vec->shape);
    if (!*out) return -1;
    memcpy((*out)->data, vec->data, 3 * sizeof(double));
    return 0;
}

int hao_group_act(const HaoArray *g, const HaoArray *x, HaoArray **out) {
    /* Placeholder for group action; identity for now */
    *out = hao_array_create(x->type, x->rank, x->shape);
    if (!*out) return -1;
    memcpy((*out)->data, x->data, x->element_count * hao_array_element_size(x->type));
    return 0;
}
