# HERMETIC-Ω (hermetic-array) · lang/ga.bqn
# SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
# License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
# Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
# Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
# Hark, though this node be but a spark,
# Its covenant endureth through the dark.

# Geometric algebra primitives pure BQN
# Geometric product ab = a·b + a∧b (simplified)

GeoProd ← {
  a ← ⊢
  b ← ⊣
  s ← Dot a b
  # Cross for 3D wedge component
  cr ← ⟕˘ (a × ⌊¯1⌋ b) - (⌊¯1⌋ a × b)
  s ⌈ cr
}

Exterior ← {
  a ← ⊢
  b ← ⊣
  # Antisymmetric part
  (GeoProd a b) - (GeoProd b a)
}

Reversion ← ⌽
Grade ← {ℓ × ⌈¯1 ⌋}
