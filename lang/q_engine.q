/ HERMETIC-Ω (hermetic-array) · lang/q_engine.q
/ SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
/ License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
/ Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
/ Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
/ Hark, though this node be but a spark,
/ Its covenant endureth through the dark.

/ HERMETIC-Ω pure Q/K vector geometric engine
/ Compact array evaluation

norm:{sqrt sum x*x}
dot:{sum x*y}
normalize:{x%norm x}

sphere_exp:{
  x:x; v:y;
  nv:sqrt sum v*v;
  c:cos nv;
  s:sin[nv]%nv;
  (c*x)+s*v
}

evolve:{
  x:x; v:y; eta:z; fuel:w;
  $[fuel=0;x;evolve[sphere_exp[x;eta*v];v;eta;fuel-1]]
}
