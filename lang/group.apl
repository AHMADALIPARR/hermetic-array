⍝ HERMETIC-Ω group operations in APL ⍞

⍝ Orbit under group action ⍞
Orbit ← {
  g ← ⍴
  x ← ⍵
  # Generate successive applications
  x, g+×x
}

⍝ Stabilizer check ⍞
Stabilizer ← {
  g ← ⍴
  x ← ⍵
  g+×x ≡ x
}

⍝ Composition ⍞
Compose ← {⍴∘⍵}
