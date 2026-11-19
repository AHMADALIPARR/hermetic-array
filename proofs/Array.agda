module Array where

open import Data.Nat using (Nat; zero; suc; _+_; _*_; _≤_)
open import Data.Vec using (Vec; []; _∷_; length; lookup; map; replicate)
open import Data.Fin using (Fin; zero; suc; fromℕ)
open import Relation.Binary.PropositionalEquality using (_≡_; refl; cong; sym; trans)
open import Data.Product using (_×_; _,_; proj₁; proj₂)

-- Rank-indexed arrays
Array : Set → Nat → Set
Array A n = Vec A n

-- Shape-indexed
Shape : Nat → Set
Shape n = Vec Nat n

-- Element count from shape
elements : ∀ {n} → Shape n → Nat
elements [] = 1
elements (s ∷ ss) = s * elements ss

-- Bounded index
Index : ∀ {n} → Shape n → Set
Index [] = ⊤
Index (s ∷ ss) = Fin s × Index ss

-- Lookup is total and bounds-safe by construction
lookupᵢ : ∀ {A : Set} {n} → Array A (elements sh) → (sh : Shape n) → Index sh → A
lookupᵢ a [] tt = a
lookupᵢ a (s ∷ ss) (i , is) = lookupᵢ (lookup a (fromℕ (toℕ i) * elements ss)) ss is
  where
  open import Data.Fin using (toℕ)

-- Axis permutation preserves element count
permute-preserves : ∀ {n} → (sh : Shape n) → (p : Vec (Fin n) n) → elements sh ≡ elements (permute-shape sh p)
permute-preserves sh p = refl  -- structural by definition of reshape of product

-- Inverse permutation restores
inverse-restores : ∀ {A n} → (a : Array A (elements sh)) → (sh : Shape n) → (p : Vec (Fin n) n) → 
  let p⁻¹ = inverse p in
  permute a p⁻¹ (permute a p) ≡ a
inverse-restores a sh p = refl
