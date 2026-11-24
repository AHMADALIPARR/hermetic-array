# HERMETIC-Ω (hermetic-array) · lang/sphere.bqn
# SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
# License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
# Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
# Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
# Hark, though this node be but a spark,
# Its covenant endureth through the dark.

# HERMETIC-Ω unit sphere exponential map in BQN
# Exp_x(v) = cos(||v||) x + sinc(||v||) v
# Zero case continuous via Taylor

Norm ← +´×˘
Dot ← +´×

Sinc ← {
  t ← ℓ
  ∣t∣ < 1e¯12 ? 1 - t×t÷6 ◇ Sin t ÷ t
}

SphereExp ← {
  x ← ⊢
  v ← ⊣
  nv ← √ Norm v
  c ← Cos nv
  s ← Sinc nv
  c×x + s×v
}

# Unit check
IsUnit ← {
  |Norm ℓ - 1| < 1e¯8
}

# Tangent check
IsTangent ← {
  |Dot ⊢ ⊣| < 1e¯8
}

# Evolution step
Evolve ← {
  x ← ⊢
  v ← ⊣
  η ← ⊤
  SphereExp x (η×v)
}
