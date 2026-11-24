<!--
  HERMETIC-Ω (hermetic-array) · docs/compatibility.md
  SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
  License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
  Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
  Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
  Hark, though this node be but a spark,
  Its covenant endureth through the dark.
-->

# HAΩ Compatibility Table

Concepts inherited, modified, or rejected from BQN, APL, and Klong.

| Concept                  | Source     | Status     | Resolution / Notes |
|--------------------------|------------|------------|--------------------|
| Tacit composition / trains | BQN      | Inherited  | Primary mechanism for point-free code. Explicit trains. |
| Function inverses        | BQN        | Inherited  | First-class `^{-1}` where mathematically valid. |
| Rank polymorphism        | BQN / APL  | Inherited  | Rank operator `¨` with explicit axis control. |
| Modifiers / combinators  | BQN / APL  | Modified   | Unified set; no duplicate glyphs. |
| Rich primitive vocabulary| APL        | Inherited  | Canonical glyph chosen per op; text fallback always available. |
| Axis-aware ops           | APL        | Inherited  | `[axis]` syntax retained. |
| Reshape, transpose, rotate, reduce, scan | APL | Inherited | Standard glyphs or text names. |
| Inner / outer products   | APL        | Inherited  | `.` and `∘.` forms. |
| Grade / permute          | APL        | Inherited  | `⍋ ⍒` and related. |
| Compact vector style     | Klong      | Modified   | Used for literals and short expressions; longer forms prefer trains. |
| Adverbs                  | Klong / APL| Modified   | Mapped onto HAΩ modifiers. |
| Recursive array processing| Klong     | Inherited  | Explicit fuel limits for termination. |
| Glyph concatenation      | —          | Rejected   | Single canonical glyph or textual name per primitive. |
| Multiple dialects        | —          | Rejected   | One grammar, one parse. |
| Implicit ambient rank    | APL        | Modified   | Rank must be explicit or inferred under strict rules. |
| Dynamic typing only      | All        | Rejected   | Static shape + refinement system (Section VI). |

All collisions resolved in favor of a single, deterministic, geometry-aware vocabulary.
