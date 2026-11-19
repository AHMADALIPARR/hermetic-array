#include "../src/array.h"
#include "../src/geom.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

int main(void) {
    size_t sh[] = {3};
    HaoArray *x = hao_array_create(HAO_SCALAR_F64, 1, sh);
    HaoArray *v = hao_array_create(HAO_SCALAR_F64, 1, sh);
    HaoArray *out = hao_array_create(HAO_SCALAR_F64, 1, sh);
    double *xd = x->data;
    double *vd = v->data;
    xd[0] = 1.0; xd[1] = 0.0; xd[2] = 0.0;
    vd[0] = 0.0; vd[1] = 0.1; vd[2] = 0.0;
    assert(hao_is_unit(xd, 3, 1e-10));
    assert(hao_is_tangent(xd, vd, 3, 1e-10));
    int rc = hao_array_sphere_exp(x, v, out);
    assert(rc == 0);
    double *od = out->data;
    double nrm = sqrt(hao_norm2(od, 3));
    assert(fabs(nrm - 1.0) < 1e-8);
    printf("sphere exp test passed\n");
    hao_array_free(x);
    hao_array_free(v);
    hao_array_free(out);
    return 0;
}
