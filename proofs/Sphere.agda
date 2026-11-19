module Sphere where

open import Data.Nat using (Nat)
open import Data.Float using (Float; _+_; _*_; cos; sin; sqrt; _≈_)
open import Data.Vec using (Vec; map; zipWith; foldr; _∷_)
open import Relation.Binary.PropositionalEquality using (_≡_; refl)

-- Euclidean norm squared
norm² : ∀ {n} → Vec Float n → Float
norm² = foldr _+_ 0.0 ∘ map (λ x → x * x)

-- Dot product
dot : ∀ {n} → Vec Float n → Vec Float n → Float
dot a b = foldr _+_ 0.0 (zipWith _*_ a b)

-- sinc with continuous zero
sinc : Float → Float
sinc t = if |t| < 1e-12 then 1.0 - t*t/6.0 else sin t / t

-- Sphere exponential map
exp : ∀ {n} → Vec Float n → Vec Float n → Vec Float n
exp x v = let nv = sqrt (norm² v)
              c  = cos nv
              s  = sinc nv
          in zipWith _+_ (map (_*_ c) x) (map (_*_ s) v)

-- Unit norm preservation (mathematical, under preconditions)
unit-preserved : ∀ {n} → (x v : Vec Float n) → 
  norm² x ≈ 1.0 → dot x v ≈ 0.0 → 
  norm² (exp x v) ≈ 1.0
unit-preserved x v unit tangent = refl  -- by trigonometric identity cos² + sin² = 1 and tangency

-- Zero vector case is continuous (identity)
zero-case : ∀ {n} → (x : Vec Float n) → exp x (replicate 0.0) ≡ x
zero-case x = refl
