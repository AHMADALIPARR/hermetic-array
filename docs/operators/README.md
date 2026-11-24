<!--
  HERMETIC-Ω (hermetic-array) · docs/operators/README.md
  SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
  License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
  Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
  Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
  Hark, though this node be but a spark,
  Its covenant endureth through the dark.
-->

# Geometric Operators (Section III)

Each operator documents:

1. Canonical glyph or textual form
2. Domain and codomain
3. Shape transformation
4. Equational semantics
5. Boundary conditions
6. ≥3 test cases
7. Error conditions

Priority: structural and metric geometry first, then geometric algebra and group operations.

## Structural Geometry

### Reshape (`⍡` or `reshape`)
- Domain: Array[A], Shape → Array[A]
- Shape: result shape is the given shape (element count must match or be filled under explicit rules)
- Semantics: classic APL reshape with exact fill rules documented
- Tests: (1) reshape 2 3 of 1..6; (2) under-fill; (3) over-fill error
- Errors: length error on mismatch without fill modifier

### Rank transformation / Axis permutation
- Glyph: `⍉` or `permute`
- Preserves element count (proof obligation 2)

### Array rotation, Reflection, Translation, Projection, Coordinate change, Broadcasting, Generalized contraction
(to be fully specified in next iteration)

## Metric Geometry

### Inner product, Norm, Normalization
### Geodesic distance, Exp, Log, Parallel transport, Curvature
(sphere Exp defined exactly as in Section V)

## Geometric Algebra

Geometric product, exterior, contractions, reversion, grade projection, rotors, sandwich.

## Group Operations

Action, orbit, stabilizer, etc.

All operators will receive full equational semantics, shape contracts, and tests before implementation in the C11 engine.
