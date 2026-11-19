#include "array.h"
#include "geom.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    size_t shape[] = {3};
    HaoArray *x = hao_array_create(HAO_SCALAR_F64, 1, shape);
    HaoArray *v = hao_array_create(HAO_SCALAR_F64, 1, shape);
    HaoArray *out = hao_array_create(HAO_SCALAR_F64, 1, shape);

    double *xd = (double *)x->data;
    double *vd = (double *)v->data;
    xd[0] = 1.0; xd[1] = 0.0; xd[2] = 0.0;
    vd[0] = 0.0; vd[1] = 0.1; vd[2] = 0.0; /* small tangent step */

    printf("Unit? %d\n", hao_is_unit(xd, 3, 1e-10));
    printf("Tangent? %d\n", hao_is_tangent(xd, vd, 3, 1e-10));

    hao_array_sphere_exp(x, v, out);
    double *od = (double *)out->data;
    printf("Exp: %f %f %f\n", od[0], od[1], od[2]);
    printf("Norm after: %f\n", sqrt(hao_norm2(od, 3)));

    hao_array_free(x);
    hao_array_free(v);
    hao_array_free(out);
    return 0;
}
