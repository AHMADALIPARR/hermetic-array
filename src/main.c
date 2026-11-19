#include "array.h"
#include "geom.h"
#include "parser.h"
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: hao <expr>\n");
        return 1;
    }
    Node *tree = parse(argv[1]);
    if (!tree) {
        fprintf(stderr, "parse failed\n");
        return 1;
    }
    IRNode *ir = ir_from_node(tree);
    HaoArray *result = NULL;
    if (ir_eval(ir, &result) == 0 && result) {
        printf("ok rank=%zu count=%zu\n", result->rank, result->element_count);
        hao_array_free(result);
    }
    node_free(tree);
    free(ir);
    hao_memory_report();
    return 0;
}
