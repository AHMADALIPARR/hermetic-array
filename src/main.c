#include "array.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    size_t sh[] = {3};
    HaoArray *x = hao_array_create(HAO_SCALAR_F64, 1, sh);
    HaoArray *v = hao_array_create(HAO_SCALAR_F64, 1, sh);
    double *xd = x->data;
    double *vd = v->data;
    xd[0]=1; xd[1]=0; xd[2]=0;
    vd[0]=0; vd[1]=0.1; vd[2]=0;

    HaoArray *normed = hao_normalize(x);
    printf("Norm: %f\n", ((double*)hao_norm(x)->data)[0]);

    HaoArray *evolved = NULL;
    hao_evolve_sphere(x, v, 1.0, 5, &evolved);
    double *ed = evolved->data;
    printf("Evolved: %f %f %f\n", ed[0], ed[1], ed[2]);
    printf("Final norm: %f\n", ((double*)hao_norm(evolved)->data)[0]);

    size_t p[] = {0};
    HaoArray *perm = NULL;
    hao_permute_axes(x, p, &perm);

    hao_array_free(x); hao_array_free(v); hao_array_free(normed); hao_array_free(evolved); hao_array_free(perm);
    return 0;
}
