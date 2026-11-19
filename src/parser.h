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
