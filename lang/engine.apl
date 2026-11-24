⍝ HERMETIC-Ω (hermetic-array) · lang/engine.apl
⍝ SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
⍝ License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
⍝ Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
⍝ Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
⍝ Hark, though this node be but a spark,
⍝ Its covenant endureth through the dark.

⍝ HERMETIC-Ω pure APL geometric array engine ⍞
⍝ Native rank, reduction, scan, products ⍞

Norm ← {√+×⍵}
Dot ← {⍴+×⍵}
Normalize ← {⍵÷Norm ⍵}

SphereExp ← {
  x ← ⍴
  v ← ⍵
  nv ← √+×v
  c ← 2○○¯1×nv
  s ← (1○¯¯1×nv)÷nv
  (c×x)+s×v
}

Inner ← {⍴+×⍵}
Outer ← {⍴∘.×⍵}

⍝ Evolution via power ⍞
Evolve ← {
  x ← ⍴
  v ← ⍵
  η ← ⍶
  n ← ⍷
  n=0:x ⋄ SphereExp(x;η×v)
}
