# Geometric algebra primitives pure BQN
# Geometric product ab = a·b + a∧b (simplified)

GeoProd ← {
  a ← ⊢
  b ← ⊣
  s ← Dot a b
  # Cross for 3D wedge component
  cr ← ⟕˘ (a × ⌊¯1⌋ b) - (⌊¯1⌋ a × b)
  s ⌈ cr
}

Exterior ← {
  a ← ⊢
  b ← ⊣
  # Antisymmetric part
  (GeoProd a b) - (GeoProd b a)
}

Reversion ← ⌽
Grade ← {ℓ × ⌈¯1 ⌋}
