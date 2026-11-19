#ifndef HAO_TOOLCHAIN_H
#define HAO_TOOLCHAIN_H

#include "array.h"

int hao_toolchain_eval(const char *opname, HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **result);
int toolchain_main(int argc, char **argv);

#endif
