# HERMETIC-Ω (hermetic-array) · tests/bqn/lang_group.bqn
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

# load: lang/group.bqn
# HERMETIC-Ω interpreter test: the original lang/group.bqn definitions

! 6‿8 ≡ Act 2 3‿4
o ← Orbit 2 1‿1 3
•Show o
! 1‿1‿2‿2‿4‿4‿8‿8 ≡ o
! 1‿1 ≡ Orbit 2 1‿1 0
! 1 ≡ Stabilizer 1 3‿4
! 0 ≡ Stabilizer 2 3‿4
! ¯3‿4 ≡ Inverse 3‿¯4
! 3‿¯4 ≡ (Inverse Compose Inverse) 3‿¯4
! 3 ≡ (Abs Compose -) 3

# agreement with the C11 group action kernels
r ← 2‿2⥊0‿¯1‿1‿0
! ¯2‿1 ≡ •hao.Act r 1‿2
! 1 ≡ •hao.Stabilizes (2‿2⥊1‿0‿0‿1) 1‿2

•Out "ok lang_group"
