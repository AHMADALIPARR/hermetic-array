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

# load:
# HERMETIC-Ω interpreter test: HAΩ glyphs and the •hao bridge to the C11 engine

Near ← {1e¯12 > ⌈´ ⥊ Abs (𝕨 - 𝕩)}

# ⊗ tensor (outer) product, ⊙ geometric product, ⟂ orthogonality
! (2‿3⥊3‿4‿5‿6‿8‿10) ≡ 1‿2 ⊗ 3‿4‿5
! 0‿0‿0‿1 ≡ 1‿0‿0 ⊙ 0‿1‿0
! 32‿¯3‿6‿¯3 ≡ •hao.GeoProd 1‿2‿3 4‿5‿6
! 1 ≡ (1‿0‿0 ⟂ 0‿1‿0)
! 0 ≡ (1‿1‿0 ⟂ 0‿1‿0)

# ⍡ exact reshape (no cycling), ↻ rotation, ⌁ geodesic midpoint
! (2‿3⥊↕6) ≡ 2‿3 ⍡ ↕6
•Show {2‿2 ⍡ 𝕩}⎊{•CurrentError 𝕩} ↕6
! 2‿3‿1 ≡ 1 ↻ 1‿2‿3
! (√0.5)‿(√0.5)‿0 Near 1‿0‿0 ⌁ 0‿1‿0

# Cl(3,0) multivectors in bitmask order 1 e1 e2 e12 e3 e13 e23 e123
e1 ← 0‿1‿0‿0‿0‿0‿0‿0
e2 ← 0‿0‿1‿0‿0‿0‿0‿0
! 0‿0‿0‿1‿0‿0‿0‿0 ≡ e1 ⊙ e2
! 0‿0‿0‿¯1‿0‿0‿0‿0 ≡ e2 ⊙ e1
! 1‿0‿0‿0‿0‿0‿0‿0 ≡ e1 ⊙ e1
! 0‿0‿0‿1‿0‿0‿0‿0 ≡ •hao.MvOuter e1 e2
! 0‿0‿0‿0‿0‿0‿0‿0 ≡ •hao.MvOuter e1 e1
! 0‿1‿2‿¯3‿4‿¯5‿¯6‿¯7 ≡ •hao.MvReverse ↕8
! 0‿0‿0‿3‿0‿5‿6‿0 ≡ •hao.MvGrade 2 (↕8)
! 0‿0‿1 ≡ •hao.Wedge 1‿0‿0 0‿1‿0

# rotors and group actions
! 0‿1‿0 Near •hao.Sandwich (•hao.Rotor 0‿0‿1 (π÷2)) 1‿0‿0
r ← 2‿2⥊0‿¯1‿1‿0
! ¯2‿1 ≡ •hao.Act r 1‿2
! (5‿2⥊1‿0‿0‿1‿¯1‿0‿0‿¯1‿1‿0) ≡ •hao.Orbit r 1‿0 4
! 1 ≡ •hao.IsSO (3‿3⥊1‿0‿0‿0‿1‿0‿0‿0‿1)
! 0 ≡ •hao.IsSO (3‿3⥊1‿0‿0‿0‿1‿0‿0‿0‿¯1)
! ¯2 ≡ •hao.Det (2‿2⥊1‿2‿3‿4)

# sphere geometry
! (π÷2) Near •hao.Distance 1‿0‿0 0‿1‿0
! 0‿(π÷2)‿0 Near •hao.Log 1‿0‿0 0‿1‿0
! 0‿0‿1 Near •hao.Transport 1‿0‿0 0‿1‿0 0‿0‿1
! ¯1‿2‿3 ≡ •hao.Reflect 1‿2‿3 1‿0‿0
! 0‿2‿3 ≡ •hao.Project 1‿0‿0 1‿2‿3
! 11 ≡ •hao.Inner 1‿2 3‿4

# linear algebra
! (2‿2⥊19‿22‿43‿50) ≡ •hao.MatMul (2‿2⥊1‿2‿3‿4) (2‿2⥊5‿6‿7‿8)
! 4 ≡ •hao.Contract (2‿2⥊1‿2‿3‿3) 0 1
! (2‿2⥊1‿2‿1‿2) ≡ •hao.Broadcast 2‿2 1‿2

•Out "ok hao_glyphs"
