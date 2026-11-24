/*
 * HERMETIC-Ω (hermetic-array) · src/ir.h
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

#ifndef HAO_IR_H
#define HAO_IR_H

#include "array.h"
#include "parser.h"

typedef enum {
    IR_LOAD,
    IR_STORE,
    IR_RESHAPE,
    IR_PERMUTE,
    IR_EXP,
    IR_DOT,
    IR_NORM,
    IR_GEO_PROD,
    IR_EVOLVE
} IROp;

typedef struct IRNode {
    IROp op;
    HaoArray *result;
    struct IRNode *args[4];
    size_t nargs;
    size_t shape[8];
    size_t rank;
} IRNode;

IRNode *ir_from_node(void *parse_node);
int ir_eval(IRNode *ir, HaoArray **out);
void ir_free(IRNode *ir);

#endif
