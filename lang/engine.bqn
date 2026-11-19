# HERMETIC-Ω pure BQN geometric array engine
# All computation via native array primitives and combinators

Norm ← +´×˘
Dot ← +´×
Normalize ← {ℓ ÷ √ Norm ℓ}

Sinc ← {
  t ← ℓ
  ∣t∣ < 1e¯12 ? 1 - (t×t)÷6 ◇ (Sin t) ÷ t
}

SphereExp ← {
  x ← ⊢
  v ← ⊣
  nv ← √ Norm v
  c ← Cos nv
  s ← Sinc nv
  (c × x) + (s × v)
}

Geodesic ← {
  x ← ⊢
  y ← ⊣
  t ← ⊤
  θ ← Arccos Dot x y
  s0 ← Sin ((1-t)×θ)
  s1 ← Sin (t×θ)
  ((s0×x) + (s1×y)) ÷ Sin θ
}

Evolve ← {
  x ← ⊢
  v ← ⊣
  η ← ⊤
  n ← ⊥
  n = 0 ? x ◇ Evolve (SphereExp x (η×v)) v η (n-1)
}

# Rank-polymorphic structural
Reshape ← ⍡
Permute ← ⍉
Rotate ← ⌽
