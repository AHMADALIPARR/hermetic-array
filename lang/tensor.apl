⍝ HERMETIC-Ω (hermetic-array) · lang/tensor.apl
⍝ SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
⍝ License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
⍝ Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
⍝ Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
⍝ Hark, though this node be but a spark,
⍝ Its covenant endureth through the dark.

⍝ Pure APL tensor and contraction operators ⍞

Tensor ← {⍴∘.×⍵}

Contract ← {
  a ← ⍴
  axes ← ⍵
  +÷[axes] a
}

⍝ Shape-preserving permute ⍞
AxisPerm ← ⍉

⍝ Broadcast ⍞
Broadcast ← {⍴⍬⍵}
