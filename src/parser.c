/* HERMETIC ARRAY Ω (HAΩ)
 * Copyright (C) 2026 AHMAD ALI PARR
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static Node *make(NodeKind k) {
    Node *n = calloc(1, sizeof(Node));
    if (!n) return NULL;
    n->kind = k;
    return n;
}

static const char *skip_ws(const char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static Node *parse_atom(const char **ps) {
    const char *s = skip_ws(*ps);
    if (!*s) return NULL;
    if (*s == '[') {
        Node *n = make(NODE_ARRAY);
        s++;
        size_t cap = 4;
        n->children = malloc(cap * sizeof(Node*));
        n->nchildren = 0;
        while (*s && *s != ']') {
            s = skip_ws(s);
            if (*s == ']') break;
            Node *child = parse_atom(&s);
            if (!child) break;
            if (n->nchildren >= cap) {
                cap *= 2;
                n->children = realloc(n->children, cap * sizeof(Node*));
            }
            n->children[n->nchildren++] = child;
            s = skip_ws(s);
            if (*s == ',') s++;
        }
        if (*s == ']') s++;
        *ps = s;
        return n;
    }
    if (isdigit((unsigned char)*s) || *s == '-' || *s == '.') {
        Node *n = make(NODE_ATOM);
        n->literal = strtod(s, (char**)&s);
        *ps = s;
        return n;
    }
    /* glyph or ident */
    Node *n = make(NODE_ATOM);
    size_t len = 0;
    while (s[len] && !isspace((unsigned char)s[len]) && s[len] != '[' && s[len] != ']' && s[len] != ',') len++;
    n->glyph = strndup(s, len);
    *ps = s + len;
    return n;
}

Node *parse(const char *src) {
    const char *s = src;
    return parse_atom(&s);
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
