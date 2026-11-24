/*
 * HERMETIC-Ω (hermetic-array) · src/main.c
 * SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
 * License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
 * Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
 * Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
 * Hark, though this node be but a spark,
 * Its covenant endureth through the dark.
 */

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

#include "array.h"
#include "geom.h"
#include "parser.h"
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    hao_pool_init(1 << 20);
    if (argc < 2) {
        fprintf(stderr, "usage: hao <expr>\n");
        hao_pool_free();
        return 1;
    }
    Node *tree = parse(argv[1]);
    if (!tree) {
        fprintf(stderr, "parse failed\n");
        hao_pool_free();
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
    hao_pool_free();
    return 0;
}
