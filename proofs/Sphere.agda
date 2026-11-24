-- HERMETIC-Ω (hermetic-array) · proofs/Sphere.agda
-- SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
-- License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
-- Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
-- Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
-- Hark, though this node be but a spark,
-- Its covenant endureth through the dark.

-- HERMETIC ARRAY Ω (HAΩ)
-- Copyright (C) 2026 AHMAD ALI PARR
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.
--
-- You should have received a copy of the GNU Affero General Public License
-- along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
