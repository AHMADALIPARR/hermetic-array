# HERMETIC-Ω (hermetic-array) · tests/bqn/lang_all.bqn
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

# load: lang/engine.bqn lang/sphere.bqn lang/types.bqn lang/group.bqn lang/ga.bqn lang/glyph_registry.bqn
# HERMETIC-Ω interpreter test: every original BQN file loads into one session

# sphere.bqn is loaded after engine.bqn, so its single-step Evolve replaces
# engine.bqn's fuel-bounded one (the files define the same name)
! (Evolve 1‿0‿0 0‿0.1‿0 2) ≡ SphereExp 1‿0‿0 0‿0.2‿0
! 25 ≡ Norm 3‿4
! 1 ≡ IsUnit UnitVector 1‿0‿0
! 6‿8 ≡ Act 2 3‿4
! 3‿2‿1 ≡ Reversion 1‿2‿3
! "rank" ≡ Registry 0

•Out "ok lang_all"
