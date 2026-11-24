# HERMETIC-Ω (hermetic-array) · tests/bqn/lang_sphere.bqn
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

# load: lang/sphere.bqn
# HERMETIC-Ω interpreter test: the original lang/sphere.bqn definitions

Near ← {1e¯12 > ⌈´ ⥊ Abs (𝕨 - 𝕩)}

x ← 1‿0‿0
v ← 0‿0.1‿0

! 1 = IsUnit x
! 0 = IsUnit 2‿0‿0
! 1 = IsTangent x v
! 0 = IsTangent x 1‿1‿0
•Show Sinc 0.5
! 0.958851077208406 Near Sinc 0.5
! 1 = Sinc 1e¯13

# one step of size η: Exp_x(η v)
s ← Evolve x v 2
•Show s
! s Near SphereExp x 0‿0.2‿0
! s Near •hao.SphereExp x 0‿0.2‿0
! IsUnit s

•Out "ok lang_sphere"
