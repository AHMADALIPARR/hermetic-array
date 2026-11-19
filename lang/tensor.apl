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
