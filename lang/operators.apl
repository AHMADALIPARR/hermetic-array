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
