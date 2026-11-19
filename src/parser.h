#ifndef HAO_PARSER_H
#define HAO_PARSER_H

#include <stddef.h>

typedef enum {
    NODE_ATOM,
    NODE_MONADIC,
    NODE_DYADIC,
    NODE_TRAIN,
    NODE_ARRAY,
    NODE_SHAPE,
    NODE_GEO
} NodeKind;

typedef struct Node {
    NodeKind kind;
    char *glyph;
    struct Node *left;
    struct Node *right;
    struct Node **children;
    size_t nchildren;
    double literal;
} Node;

Node *parse(const char *src);
void node_free(Node *n);

#endif
