#include "array.h"
#include "geom.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--repl") == 0) {
        printf("HAΩ REPL (minimal)\n");
        char line[256];
        while (fgets(line, sizeof(line), stdin)) {
            printf("parsed: %s", line);
        }
        return 0;
    }

    size_t sh[] = {3};
    HaoArray *x = hao_array_create(HAO_SCALAR_F64, 1, sh);
    HaoArray *v = hao_array_create(HAO_SCALAR_F64, 1, sh);
    double *xd = x->data;
    double *vd = v->data;
    xd[0] = 1.0; xd[1] = 0.0; xd[2] = 0.0;
    vd[0] = 0.0; vd[1] = 0.1; vd[2] = 0.0;

    printf("Unit norm: %f\n", ((double*)hao_norm(x)->data)[0]);
    printf("Dot: %f\n", hao_inner(x, v));

    HaoArray *evolved = NULL;
    int rc = hao_evolve_sphere(x, v, 1.0, 3, &evolved);
    if (rc == 0) {
        double *ed = evolved->data;
        printf("Evolved: %f %f %f (norm %f)\n", ed[0], ed[1], ed[2], ((double*)hao_norm(evolved)->data)[0]);
    } else {
        printf("Evolve failed: %d\n", rc);
    }

    size_t perm[] = {2, 0, 1};
    HaoArray *pout = NULL;
    hao_permute_axes(x, perm, &pout);
    printf("Permute done\n");

    hao_array_free(x);
    hao_array_free(v);
    if (evolved) hao_array_free(evolved);
    if (pout) hao_array_free(pout);
    return 0;
}
