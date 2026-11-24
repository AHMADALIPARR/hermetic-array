# HERMETIC-Ω (hermetic-array) · tests/bqn/lang_types.bqn
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

# load: lang/sphere.bqn lang/types.bqn
# HERMETIC-Ω interpreter test: the original lang/types.bqn definitions

Near ← {1e¯12 > ⌈´ ⥊ Abs (𝕨 - 𝕩)}

x ← 1‿0‿0
v ← 0‿0.1‿0

# refinement types: pass the value through or raise
! x ≡ UnitVector x
! "not unit" ≡ {UnitVector 𝕩}⎊{•CurrentError 𝕩} 2‿0‿0
! v ≡ Tangent x v
! "not tangent" ≡ {Tangent x 𝕩}⎊{•CurrentError 𝕩} 1‿1‿0

# Geodesic as written: under HAΩ precedence `Sin (1-t)×θ` reads (Sin (1-t))×θ,
# so the midpoint is not the slerp midpoint (see docs/interpreter.md)
g ← Geodesic 1‿0‿0 0‿1‿0 0.5
•Show g
a ← (Sin 0.5)×π÷2
! g Near ⟨a, a, 0⟩
! (√0.5)‿(√0.5)‿0 Near •hao.Geodesic 1‿0‿0 0‿1‿0 0.5

# Rotate as written is elementwise: a 4-quaternion against a 3-vector does not agree
•Show {Rotate 1‿0‿0‿0 𝕩}⎊{•CurrentError 𝕩} 1‿2‿3
! 1‿0‿0‿0 ≡ Rotate 1‿0‿0‿0 1‿2‿3‿4
! 1‿¯2‿¯3‿¯4 ≡ Conj 1‿2‿3‿4
# the C engine's sandwich rotates 1‿0‿0 by 90° about z
! 0‿1‿0 Near •hao.Sandwich (•hao.Rotor 0‿0‿1 (π÷2)) 1‿0‿0

•Out "ok lang_types"
