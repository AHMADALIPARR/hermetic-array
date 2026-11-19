# Pure BQN group operations
# Action, orbit, stabilizer via array combinators

Act ← {
  g ← ⊢
  x ← ⊣
  g × x
}

Orbit ← {
  g ← ⊢
  x ← ⊣
  n ← ⊤
  n = 0 ? x ◇ x ⌈ Orbit g (Act g x) (n-1)
}

Stabilizer ← {
  g ← ⊢
  x ← ⊣
  Act g x ≡ x
}

Compose ← ∘
Inverse ← {ℓ × ⌊¯1⌋}
