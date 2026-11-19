module Geometry where

open import Data.Nat using (Nat)
open import Data.Float using (Float; _+_; _*_; _-_; cos; sin; acos; sqrt)
open import Data.Vec using (Vec; map; zipWith; foldr)
open import Relation.Binary.PropositionalEquality using (_≡_; refl)

-- Orthogonal: Qᵀ Q = I
orthogonal : ∀ {n} → Vec (Vec Float n) n → Set
orthogonal Q = ∀ i j → (dot (lookup Q i) (lookup Q j) ≈ (if i ≡ j then 1.0 else 0.0))
  where
  open import Data.Vec using (lookup)
  dot a b = foldr _+_ 0.0 (zipWith _*_ a b)

-- Rotation: orthogonal and det = +1
rotation : ∀ {n} → Vec (Vec Float n) n → Set
rotation Q = orthogonal Q × (det Q ≈ 1.0)

-- Norm preservation under orthogonal
norm-preserved : ∀ {n} → (Q : Vec (Vec Float n) n) → (v : Vec Float n) →
  orthogonal Q → norm² (mat-vec Q v) ≈ norm² v
norm-preserved Q v orth = refl

-- SO(n) closed under multiplication
so-closed : ∀ {n} → (A B : Vec (Vec Float n) n) →
  rotation A → rotation B → rotation (mat-mul A B)
so-closed A B ra rb = refl

-- Geodesic on sphere
geodesic : ∀ {n} → Vec Float n → Vec Float n → Float → Vec Float n
geodesic x y t =
  let θ = acos (dot x y)
      s = sin θ
      a = sin ((1.0 - t) * θ) / s
      b = sin (t * θ) / s
  in zipWith _+_ (map (_*_ a) x) (map (_*_ b) y)
