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

# load: lang/engine.bqn
# HERMETIC-Ω interpreter test: the original lang/engine.bqn definitions

Near ← {1e¯12 > ⌈´ ⥊ Abs (𝕨 - 𝕩)}

x ← 1‿0‿0
v ← 0‿0.1‿0

•Show Norm 3‿4
! 25 = Norm 3‿4
! 32 = Dot ⟨1,2,3⟩ ⟨4,5,6⟩
•Show Normalize 3‿4
! 0.6‿0.8 Near Normalize 3‿4

# continuous sinc: Taylor branch at 0, sin t ÷ t elsewhere
! 1 = Sinc 0
! ((Sin 0.5) ÷ 0.5) Near Sinc 0.5

# Exp_x(v) stays on the sphere and agrees with the C11 kernel
y ← SphereExp x v
•Show y
! 1‿1 Near ⟨Norm y, •hao.Norm y⟩
! y Near •hao.SphereExp x v

# slerp midpoint of two orthogonal unit vectors
•Show Geodesic 1‿0‿0 0‿1‿0 0.5
! (Geodesic 1‿0‿0 0‿1‿0 0.5) Near (√0.5)‿(√0.5)‿0

# fuel-bounded recursive evolution matches hao_evolve_sphere step for step
e ← Evolve 1‿0‿0 0‿0.05‿0 1 3
•Show e
! e Near •hao.Evolve 1‿0‿0 0‿0.05‿0 1 3
! x ≡ Evolve x v 1 0

# structural aliases
! (2‿3⥊↕6) ≡ 2‿3 Reshape ↕6
! (3‿2⥊0‿3‿1‿4‿2‿5) ≡ Permute (2‿3 Reshape ↕6)
! 2‿3‿1 ≡ 1 Rotate 1‿2‿3
! 3‿2‿1 ≡ Rotate 1‿2‿3

•Out "ok lang_engine"
