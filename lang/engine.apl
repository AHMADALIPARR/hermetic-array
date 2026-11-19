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
