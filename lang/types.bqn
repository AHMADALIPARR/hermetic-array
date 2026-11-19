# HERMETIC-Ω type refinements in BQN
# UnitVector, Tangent, Rotor

UnitVector ← {
  v ← ℓ
  IsUnit v ? v ◇ ⍣"not unit"
}

Tangent ← {
  base ← ⊢
  v ← ⊣
  IsTangent base v ? v ◇ ⍣"not tangent"
}

# Geodesic interpolation
Geodesic ← {
  x ← ⊢
  y ← ⊣
  t ← ⊤
  θ ← Arccos Dot x y
  s0 ← Sin (1-t)×θ
  s1 ← Sin t×θ
  (s0×x + s1×y) ÷ Sin θ
}

# Group action (rotation)
Rotate ← {
  q ← ⊢
  v ← ⊣
  # Quaternion sandwich simplified
  q × v × Conj q
}
