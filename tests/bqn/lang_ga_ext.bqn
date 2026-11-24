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

# load: lang/engine.bqn lang/ga.bqn lang/ga_ext.bqn
# HERMETIC-Ω interpreter test: lang/ga_ext.bqn completes ga.bqn's GeoProd without ⟕

Near ← {1e¯12 > ⌈´ ⥊ Abs (𝕨 - 𝕩)}

! 0‿0‿1 ≡ Cross 1‿0‿0 0‿1‿0
! 1‿0‿0 ≡ Cross 0‿1‿0 0‿0‿1
! ¯3‿6‿¯3 ≡ Cross 1‿2‿3 4‿5‿6
g ← GeoProd3 1‿2‿3 4‿5‿6
•Show g
! 32‿¯3‿6‿¯3 ≡ g
# agrees with the C11 kernel hao_geometric_product3
! g ≡ •hao.GeoProd 1‿2‿3 4‿5‿6
! (1‿0‿0 ⊙ 0‿1‿0) ≡ GeoProd3 1‿0‿0 0‿1‿0
! 0‿¯6‿12‿¯6 ≡ Exterior3 1‿2‿3 4‿5‿6
# a ∧ a = 0
! 0‿0‿0‿0 ≡ Exterior3 1‿2‿3 1‿2‿3

•Out "ok lang_ga_ext"
