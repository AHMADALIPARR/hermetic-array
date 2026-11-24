NB. HERMETIC-Ω (hermetic-array) · lang/engine.j
NB. SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
NB. License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
NB. Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
NB. Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
NB. Hark, though this node be but a spark,
NB. Its covenant endureth through the dark.

NB. HERMETIC-Ω pure J tacit geometric engine
NB. Rank conjunctions, forks, hooks

norm =: +/ . *
dot =: +/ . *
normalize =: ] % %: norm

sphere_exp =: 3 : 0
  'x v' =. y
  nv =. %: norm v
  c =. 2 o. nv
  s =. (1 o. nv) % nv
  (c * x) + (s * v)
)

geodesic =: 3 : 0
  'x y t' =. y
  theta =. _2 o. dot x,y
  s0 =. 1 o. (1-t)*theta
  s1 =. 1 o. t*theta
  ((s0*x) + (s1*y)) % 1 o. theta
)

NB. Recursive evolution with power
evolve =: 4 : 0
  'x v eta n' =. y
  n = 0 do. x return. end.
  sphere_exp (x ; (eta*v))
)
