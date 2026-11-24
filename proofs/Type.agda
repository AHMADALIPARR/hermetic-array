-- HERMETIC-Ω (hermetic-array) · proofs/Type.agda
-- SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
-- License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
-- Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
-- Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
-- Hark, though this node be but a spark,
-- Its covenant endureth through the dark.

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
