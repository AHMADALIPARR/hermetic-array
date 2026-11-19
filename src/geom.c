#include "geom.h"
#include "array.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

int hao_geometric_product(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (!a || !b || a->type != HAO_SCALAR_F64 || b->type != HAO_SCALAR_F64) return -1;
    size_t n = a->element_count;
    if (n != b->element_count || n > 4) return -2; /* limited to R^3 + scalar for now */
    *out = hao_array_create(HAO_SCALAR_F64, a->rank, a->shape);
    if (!*out) return -1;
    const double *ad = a->data, *bd = b->data;
    double *od = (*out)->data;
    /* Geometric product for multivectors in Cl(3): a·b + a∧b */
    double scalar = 0.0;
    for (size_t i = 0; i < n && i < 3; i++) scalar += ad[i] * bd[i];
    od[0] = scalar;
    if (n >= 3) {
        od[1] = ad[1]*bd[2] - ad[2]*bd[1]; /* e1 component of cross */
        od[2] = ad[2]*bd[0] - ad[0]*bd[2];
        if (n > 3) od[3] = ad[0]*bd[1] - ad[1]*bd[0];
    }
    return 0;
}

int hao_exterior(const HaoArray *a, const HaoArray *b, HaoArray **out) {
    if (!a || !b) return -1;
    int rc = hao_geometric_product(a, b, out);
    if (rc != 0) return rc;
    /* Zero the scalar part for pure exterior */
    if ((*out)->element_count > 0) ((double *)(*out)->data)[0] = 0.0;
    return 0;
}

int hao_sandwich(const HaoArray *rotor, const HaoArray *vec, HaoArray **out) {
    if (!rotor || !vec || vec->element_count != 3 || rotor->element_count < 4) return -1;
    /* Simplified: assume unit rotor, compute R v R~ (identity for demo if not full) */
    *out = hao_array_create(HAO_SCALAR_F64, vec->rank, vec->shape);
    if (!*out) return -1;
    memcpy((*out)->data, vec->data, 3 * sizeof(double));
    /* Full sandwich requires rotor reversion and products; kept minimal here */
    return 0;
}

int hao_group_act(const HaoArray *g, const HaoArray *x, HaoArray **out) {
    if (!g || !x) return -1;
    /* Identity action as baseline; real group would apply transformation */
    *out = hao_array_create(x->type, x->rank, x->shape);
    if (!*out) return -1;
    if (x->element_count > 0)
        memcpy((*out)->data, x->data, x->element_count * hao_array_element_size(x->type));
    return 0;
}
