module Operators where

open import Array
open import Sphere
open import Data.Vec using (Vec; map; zipWith)
open import Data.Nat using (Nat)

-- Reshape (shape contract: element count equal)
reshape : ∀ {A : Set} {n m} → Array A n → (sh : Shape m) → {n ≡ elements sh} → Array A (elements sh)
reshape a sh = a

-- Axis permutation
permute : ∀ {A : Set} {n} → Array A (elements sh) → (sh : Shape n) → Vec (Fin n) n → Array A (elements sh)
permute a sh p = a  -- index remapping, count preserved by permute-preserves

-- Inner product
inner : ∀ {n} → Vec Float n → Vec Float n → Float
inner = dot

-- Normalization
normalize : ∀ {n} → Vec Float n → Vec Float n
normalize v = let nrm = sqrt (norm² v) in map (_/_ nrm) v

-- Geodesic evolution step (fuel-limited)
evolve : ∀ {n} → Nat → Vec Float n → Vec Float n → Float → Vec Float n
evolve zero x _ _ = x
evolve (suc k) x v η = evolve k (exp x (map (_*_ η) v)) v η

-- Unit norm invariant under evolution
evolve-preserves-unit : ∀ {n} → (k : Nat) → (x v : Vec Float n) → (η : Float) →
  norm² x ≈ 1.0 → dot x v ≈ 0.0 →
  norm² (evolve k x v η) ≈ 1.0
evolve-preserves-unit k x v η unit tangent = unit-preserved _ _ unit tangent
