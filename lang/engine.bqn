# HERMETIC-Ω (hermetic-array) · lang/engine.bqn
# SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
# License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
# Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
# Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
# Hark, though this node be but a spark,
# Its covenant endureth through the dark.

# HERMETIC ARRAY Ω (HAΩ)
# Copyright (C) 2026 AHMAD ALI PARR
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# HERMETIC-Ω pure BQN geometric array engine
# All computation via native array primitives and combinators

Norm ← +´×˘
Dot ← +´×
Normalize ← {ℓ ÷ √ Norm ℓ}

Sinc ← {
  t ← ℓ
  ∣t∣ < 1e¯12 ? 1 - (t×t)÷6 ◇ (Sin t) ÷ t
}

SphereExp ← {
  x ← ⊢
  v ← ⊣
  nv ← √ Norm v
  c ← Cos nv
  s ← Sinc nv
  (c × x) + (s × v)
}

Geodesic ← {
  x ← ⊢
  y ← ⊣
  t ← ⊤
  θ ← Arccos Dot x y
  s0 ← Sin ((1-t)×θ)
  s1 ← Sin (t×θ)
  ((s0×x) + (s1×y)) ÷ Sin θ
}

Evolve ← {
  x ← ⊢
  v ← ⊣
  η ← ⊤
  n ← ⊥
  n = 0 ? x ◇ Evolve (SphereExp x (η×v)) v η (n-1)
}

# Rank-polymorphic structural
Reshape ← ⍡
Permute ← ⍉
Rotate ← ⌽
