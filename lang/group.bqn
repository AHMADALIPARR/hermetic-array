# HERMETIC-Ω (hermetic-array) · lang/group.bqn
# SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
# License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
# Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
# Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
# Hark, though this node be but a spark,
# Its covenant endureth through the dark.

# Pure BQN group operations
# Action, orbit, stabilizer via array combinators

Act ← {
  g ← ⊢
  x ← ⊣
  g × x
}

Orbit ← {
  g ← ⊢
  x ← ⊣
  n ← ⊤
  n = 0 ? x ◇ x ⌈ Orbit g (Act g x) (n-1)
}

Stabilizer ← {
  g ← ⊢
  x ← ⊣
  Act g x ≡ x
}

Compose ← ∘
Inverse ← {ℓ × ⌊¯1⌋}
