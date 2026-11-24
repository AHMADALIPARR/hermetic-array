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

/* HERMETIC-Ω IR completion (additions only).
   Supplies the missing body of ir_free (declared in ir.h), a lowering that
   materializes array literals into IR_LOAD nodes, and an evaluator that
   dispatches every IROp to a real kernel. ir_from_node / ir_eval in ir.c
   are left untouched. */

#ifndef HAO_IR_EXT_H
#define HAO_IR_EXT_H

#include "ir.h"

IRNode *ir_node_new(IROp op, size_t nargs, IRNode **args);
IRNode *ir_load_array(const HaoArray *a);
IRNode *ir_from_node_ext(const Node *pn);
int ir_eval_ext(const IRNode *ir, HaoArray **out);

#endif
