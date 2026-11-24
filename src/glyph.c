/*
 * HERMETIC-Ω (hermetic-array) · src/glyph.c
 * SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
 * License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
 * Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
 * Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
 * Hark, though this node be but a spark,
 * Its covenant endureth through the dark.
 */

#include <string.h>
#include <stdio.h>

typedef struct {
    const char *glyph;
    const char *source;
    const char *hao_meaning;
    int arity;
    const char *rank_behavior;
} GlyphEntry;

static const GlyphEntry registry[] = {
    {"⊗", "APL tensor", "tensor product", 2, "rank sum"},
    {"∧", "GA exterior", "wedge product", 2, "grade add"},
    {"∘", "BQN composition", "function composition", 2, "n/a"},
    {"⟂", "geometry", "orthogonality predicate", 2, "scalar"},
    {"⊙", "new", "geometric product variant", 2, "same rank"},
    {"↻", "new", "rotation", 2, "axis"},
    {"⌁", "new", "geodesic interpolation", 2, "same shape"},
    {"⍉", "APL transpose", "axis permutation", 1, "rank preserve"},
    {"⌽", "APL reverse", "axis reversal", 1, "rank preserve"},
    {"⍤", "BQN rank", "rank-derived application", 2, "rank control"},
    {NULL, NULL, NULL, 0, NULL}
};

int glyph_lookup(const char *g, GlyphEntry *out) {
    for (int i = 0; registry[i].glyph; i++) {
        if (strcmp(registry[i].glyph, g) == 0) {
            *out = registry[i];
            return 0;
        }
    }
    return -1;
}
