module Type where

open import Data.Nat using (Nat)
open import Data.Vec using (Vec)

data Scalar : Set where
  F64 : Scalar
  I64 : Scalar

data ArrayType : Set where
  arr : Scalar → Nat → ArrayType
  unit-vector : Nat → ArrayType
  tangent : Nat → ArrayType
  rotor : Nat → ArrayType

-- Refinements are statically tracked
record UnitVector (n : Nat) : Set where
  field
    vec : Vec Float n
    unit : norm² vec ≈ 1.0

record TangentVector (n : Nat) : Set where
  field
    base : UnitVector n
    vec : Vec Float n
    tangent : dot (UnitVector.vec base) vec ≈ 0.0

-- Invalid combinations rejected at type level
