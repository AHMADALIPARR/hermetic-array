# HERMETIC-Ω unit sphere exponential map in BQN
# Exp_x(v) = cos(||v||) x + sinc(||v||) v
# Zero case continuous via Taylor

Norm ← +´×˘
Dot ← +´×

Sinc ← {
  t ← ℓ
  ∣t∣ < 1e¯12 ? 1 - t×t÷6 ◇ Sin t ÷ t
}

SphereExp ← {
  x ← ⊢
  v ← ⊣
  nv ← √ Norm v
  c ← Cos nv
  s ← Sinc nv
  c×x + s×v
}

# Unit check
IsUnit ← {
  |Norm ℓ - 1| < 1e¯8
}

# Tangent check
IsTangent ← {
  |Dot ⊢ ⊣| < 1e¯8
}

# Evolution step
Evolve ← {
  x ← ⊢
  v ← ⊣
  η ← ⊤
  SphereExp x (η×v)
}
