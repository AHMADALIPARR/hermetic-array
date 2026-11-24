# HERMETIC ARRAY Ω (HAΩ)
# Copyright (C) 2026 AHMAD ALI PARR
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# load:
# flags: -b
# HERMETIC-Ω interpreter test: standard BQN primitives in strict mode (-b).
# This file is plain BQN and is also run under CBQN as the reference.

Near ← {1e¯12 > ⌈´ ⥊ |𝕨 - 𝕩}

# arithmetic, right to left
! 14 ≡ 2 × 3 + 4
! 2‿4‿6 ≡ 2 × 1‿2‿3
! (2‿3⥊1‿2‿3‿5‿6‿7) ≡ (2‿3⥊↕6) + 1‿2
! 8 ≡ 2 ⋆ 3
! 3 ≡ √ 9
! ¯3 ≡ ⌊ ¯2.5
! 4 ≡ ⌈ 3.2
! 5 ≡ | ¯5
! 1 ≡ 3 | 7
! 0‿1 ≡ ¬ 1‿0
! 1‿0‿0 ≡ 1‿2‿3 < 2

# structure
! ⟨0,1,2,3⟩ ≡ ↕4
! 2‿3 ≡ ≢ 2‿3⥊0
! 6 ≡ ≠ "abcdef"
! (2‿3⥊0‿1‿2‿0‿1‿2) ≡ 2‿3 ⥊ ↕3
! 1‿2‿3‿4 ≡ 1‿2 ∾ 3‿4
! (2‿2⥊1‿2‿3‿4) ≡ 1‿2 ≍ 3‿4
! ⟨1‿2, 3⟩ ≡ 1‿2 ⋈ 3
! 1‿2 ≡ 2 ↑ 1‿2‿3
! ⟨3⟩ ≡ 2 ↓ 1‿2‿3
! 3‿2‿1 ≡ ⌽ 1‿2‿3
! 2‿3‿1 ≡ 1 ⌽ 1‿2‿3
! (3‿2⥊0‿3‿1‿4‿2‿5) ≡ ⍉ 2‿3⥊↕6
! 0‿0‿2 ≡ 2‿0‿1 / 0‿1‿2
! 1‿2‿0 ≡ ⍋ 3‿1‿2
! 0‿2‿1 ≡ ⍒ 3‿1‿2
! 1‿2‿3 ≡ ∧ 3‿1‿2
! "ca" ≡ 2‿0 ⊏ "abc"
! 'b' ≡ 1 ⊑ "abc"
! 1‿3 ≡ "abc" ⊐ "bx"
! 1‿0‿1 ≡ "aba" ∊ "a"
! 0‿1‿0 ≡ "ab" ⍷ "xaby"
! 0‿1‿0‿2 ≡ ⊒ "aaba"
! ⟨⟨0,2⟩,⟨1⟩⟩ ≡ 0‿1‿0 ⊔ ↕3
! 2‿3‿0 ≡ « 1‿2‿3
! 0‿1‿2 ≡ » 1‿2‿3
! "zab" ≡ "ab" « "xyz"
! "abx" ≡ "ab" » "xyz"

# modifiers
! 10 ≡ +´ 1‿2‿3‿4
! 1‿3‿6‿10 ≡ +` 1‿2‿3‿4
! 2‿4‿6 ≡ 2 ×¨ 1‿2‿3
! (2‿3⥊1‿2‿3‿2‿4‿6) ≡ 1‿2 ×⌜ 1‿2‿3
! 4‿6 ≡ +˝ 2‿2⥊1‿2‿3‿4
! 3‿7 ≡ +´˘ 2‿2⥊1‿2‿3‿4
! 9 ≡ ×˜ 3
! ¯1 ≡ 2 -˜ 1
! 3 ≡ 3 ⊣ 2 ⊢ 2
! 5 ≡ 5˙ 7
! ¯3 ≡ -∘⌊ 3.5
! 6 ≡ 2 +○(×˜) 1.4142135623730951 + 0
! ¯1 ≡ 3 (-⊸+) 2
! 1 ≡ 3 (+⟜-) 2
! 16 ≡ ×˜⍟2 2
! 3 ≡ {𝕩+1}⊘{𝕨+𝕩} 2
! 5 ≡ 3 {𝕩+1}⊘{𝕨+𝕩} 2
! "caught" ≡ {𝕩 ⊑ ↕2}⎊"caught" 5
! 1‿2‿10 ≡ 10⌾(2⊸⊑) 1‿2‿3
! (2‿3⥊0‿1‿2‿10‿11‿12) ≡ 0‿10 +⎉(0‿1) ↕3

# trains and blocks
Avg ← +´ ÷ ≠
! 2.5 ≡ Avg 1‿2‿3‿4
! 1‿4‿9 ≡ (×˜) 1‿2‿3
Fact ← {𝕩 ≤ 1 ? 1 ; 𝕩 × 𝕊 𝕩 - 1}
! 120 ≡ Fact 5
! 7 ≡ 3 {𝕨 + 2 × 𝕩} 2
! 0.5 Near 1 ÷ 2

# •Repr gives BQN source text
! "¯1.5" ≡ •Repr ¯1.5
! "1‿2‿3" ≡ •Repr 1‿2‿3
! "⟨1‿2,""x""⟩" ≡ •Repr ⟨1‿2,"x"⟩
! "(2‿3⥊0‿1‿2‿3‿4‿5)" ≡ •Repr 2‿3⥊↕6
! "(<1)" ≡ •Repr <1
! "1e¯20" ≡ •Repr 1e¯20

•Out "ok bqn_prims"
