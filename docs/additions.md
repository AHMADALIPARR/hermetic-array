<!--
  HERMETIC-Ω (hermetic-array) · docs/additions.md
  SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
  License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
  Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
  Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
  Hark, though this node be but a spark,
  Its covenant endureth through the dark.
-->

# HAΩ Additions

Everything listed here is new. No existing line in the repository was changed.
Where an original function is incomplete or wrong, a separate function sits
beside it and the original is left untouched.

## What was missing

| Original | State | Addition |
|----------|-------|----------|
| `ir_free` (declared in `ir.h`) | never defined | `src/ir_ext.c` |
| `ir_eval` DOT / NORM / RESHAPE / PERMUTE | return 0 without computing anything | `ir_eval_ext` |
| `ir_eval` IR_LOAD / EXP | IR_LOAD returns -1; EXP is passed a NULL `*out` | `ir_eval_ext` |
| no conversion from parser to IR | — | `ir_from_node_ext`, `ir_load_array` |
| `hao_permute_axes` | uses the inverse permutation; writes out of bounds when the permutation is not an involution (ASan: heap-buffer-overflow at `src/array.c:207` for perm 1‿2‿0) | `hao_transpose_axes` |
| `hao_geometric_product` | drops the e12 component; output uninitialised for n < 3 | `hao_geometric_product3`, `hao_mv_product` (full Cl(3,0)) |
| `hao_exterior` | alias of the geometric product | `hao_wedge3`, `hao_mv_outer` |
| `hao_quat_rotate` | aliasing in `hao_quat_mul(tmp, qinv, tmp)`: a 90° turn about z sends 1‿0‿0 to 0‿0.5‿0 | `hao_rotor_apply`, `hao_quat_sandwich` give 0‿1‿0 |
| `hao_sandwich` / `hao_group_act` | a copy placeholder and the identity | `hao_rotor_sandwich`, `hao_group_act_matrix`, `hao_orbit`, `hao_stabilizes` |
| array primitives (iota, reverse, take, drop, join, reduce, scan, outer, matmul, grade, broadcast, …) | absent | `src/array_ext.c` |
| sphere geometry (log map, distance, transport, projection), Euclidean transforms | absent | `src/geom_ext.c` |
| `ga.bqn` `GeoProd` / `Exterior` | use `⟕`, which nothing defines | `lang/ga_ext.bqn`: `Cross`, `GeoProd3`, `Exterior3` |
| a runtime for `lang/*.bqn` | CBQN rejects every file | `hao-bqn` interpreter (`docs/interpreter.md`) |

## Build notes

- The original `make` stops at `src/parser.c:82`: `strndup` is not declared under `-std=c11`
  (an error on GCC 14, a pointer-truncating warning on CompCert).
  `Makefile.ext` adds `-D_POSIX_C_SOURCE=200809L`.
- `src/glyph.c` is not in the original Makefile.
  `OP_GEODESIC` is never dispatched, and `toolchain_main` is never called.
- CompCert gives warnings only on original code: `_Alignas` in `array.h:57` and the unused `g` in `geom.c:154`.
- New code avoids the features CompCert lacks: struct passing, `long double`
  (so no `isnan`/`isinf`), and the `INFINITY`/`HUGE_VAL` literals.
