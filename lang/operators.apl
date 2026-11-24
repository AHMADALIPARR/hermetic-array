⍝ HERMETIC-Ω (hermetic-array) · lang/operators.apl
⍝ SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
⍝ License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
⍝ Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
⍝ Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
⍝ Hark, though this node be but a spark,
⍝ Its covenant endureth through the dark.

⍝ HERMETIC-Ω geometric operators in APL ⍞
⍝ Reshape, permute, inner, norm, exp ⍞

⍝ Reshape ⍞
Reshape ← {⍴⍡⍵}

⍝ Axis permute (simple) ⍞
Permute ← {⍴⍉⍵}

⍝ Inner product ⍞
Inner ← {⍴+×⍵}

⍝ Norm ⍞
Norm ← {√+×⍵}

⍝ Normalize ⍞
Normalize ← {⍵÷Norm ⍵}

⍝ Sphere exp (vector) ⍞
SphereExp ← {
  x ← ⍴
  v ← ⍵
  nv ← √+×v
  c ← 2○○¯1×nv
  s ← (1○¯¯1×nv)÷nv
  (c×x) + s×v
}

⍝ Geometric product simplified 3D ⍞
GeoProd ← {
  a ← ⍴
  b ← ⍵
  s ← +×a b
  cr ← (a[2]×b[3]) - a[3]×b[2]
  (s,cr)
}
