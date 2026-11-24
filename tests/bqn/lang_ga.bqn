# HERMETIC-Ω (hermetic-array) · tests/bqn/lang_ga.bqn
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

# load: lang/engine.bqn lang/ga.bqn
# HERMETIC-Ω interpreter test: the original lang/ga.bqn definitions

! 3‿2‿1 ≡ Reversion 1‿2‿3
! ¯1‿¯2‿¯3 ≡ Grade 1‿2‿3

# GeoProd and Exterior are written with ⟕, which no BQN dialect (nor the
# repo's glyph registry) defines; the interpreter reports it instead of guessing
e ← {GeoProd 1‿0‿0 𝕩}⎊{•CurrentError 𝕩} 0‿1‿0
•Show e
msg ← "undefined primitive ⟕"
! msg ≡ (≠msg) ↑ e
! msg ≡ (≠msg) ↑ {Exterior 1‿0‿0 𝕩}⎊{•CurrentError 𝕩} 0‿1‿0

•Out "ok lang_ga"
