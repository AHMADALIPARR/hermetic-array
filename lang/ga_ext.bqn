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

Cross ← {
  a ← ⊢
  b ← ⊣
  # 3D cross product from cyclic rotations (dual of the wedge a∧b)
  (⌊1⌋ a × ⌊¯1⌋ b) - (⌊¯1⌋ a × ⌊1⌋ b)
}

GeoProd3 ← {
  a ← ⊢
  b ← ⊣
  s ← Dot a b
  # Scalar part then bivector part ⟨e23 e31 e12⟩
  cr ← Cross a b
  s ⌈ cr
}

Exterior3 ← {
  a ← ⊢
  b ← ⊣
  # Antisymmetric part
  (GeoProd3 a b) - (GeoProd3 b a)
}
