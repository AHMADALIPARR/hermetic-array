# HERMETIC-Ω (hermetic-array) · lang/types.bqn
# SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
# License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
# Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
# Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
# Hark, though this node be but a spark,
# Its covenant endureth through the dark.

# HERMETIC-Ω type refinements in BQN
# UnitVector, Tangent, Rotor

UnitVector ← {
  v ← ℓ
  IsUnit v ? v ◇ ⍣"not unit"
}

Tangent ← {
  base ← ⊢
  v ← ⊣
  IsTangent base v ? v ◇ ⍣"not tangent"
}

# Geodesic interpolation
Geodesic ← {
  x ← ⊢
  y ← ⊣
  t ← ⊤
  θ ← Arccos Dot x y
  s0 ← Sin (1-t)×θ
  s1 ← Sin t×θ
  (s0×x + s1×y) ÷ Sin θ
}

# Group action (rotation)
Rotate ← {
  q ← ⊢
  v ← ⊣
  # Quaternion sandwich simplified
  q × v × Conj q
}
