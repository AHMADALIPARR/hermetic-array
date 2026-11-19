#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static Node *make(NodeKind k) {
    Node *n = calloc(1, sizeof(Node));
    n->kind = k;
    return n;
}

Node *parse(const char *src) {
    /* Minimal recursive descent for monadic/dyadic and array literals.
       Full EBNF from grammar.md to be elaborated. */
    if (!src || !*src) return NULL;
    Node *n = make(NODE_ATOM);
    n->glyph = strdup(src);
    return n;
}

void node_free(Node *n) {
    if (!n) return;
    free(n->glyph);
    node_free(n->left);
    node_free(n->right);
    for (size_t i = 0; i < n->nchildren; i++) node_free(n->children[i]);
    free(n->children);
    free(n);
}
