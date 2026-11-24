<!--
  HERMETIC-Ω (hermetic-array) · docs/grammar.md
  SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
  License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
  Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
  Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
  Hark, though this node be but a spark,
  Its covenant endureth through the dark.
-->

# HAΩ Canonical Grammar (EBNF)

One coherent surface syntax. Every accepted program has exactly one parse.

## Lexical rules

```ebnf
letter     = "A".."Z" | "a".."z" | "_" ;
ident      = letter , { letter | digit } ;
digit      = "0".."9" ;
integer    = digit , { digit } ;
float      = integer , "." , { digit } ;
string     = '"' , { any - '"' } , '"' ;
glyph      = (* Unicode mathematical / array glyphs reserved for primitives *) ;
whitespace = { " " | "\t" | "\n" | "\r" } ;
comment    = "#" , { any - "\n" } , "\n" ;
```

## Top-level

```ebnf
program    = { statement } ;
statement  = binding | expression ;
binding    = ident , ":" , expression , ";" ;
```

## Expressions (precedence low to high)

```ebnf
expression = train | composition | application | atom ;

train      = expression , { " " , expression } ;  (* BQN-style trains *)
composition= expression , "∘" , expression ;         (* or ○ for tacit *)

application= monadic | dyadic ;
monadic    = glyph_or_ident , expression ;
dyadic     = expression , glyph_or_ident , expression ;

glyph_or_ident = glyph | ident ;

atom       = literal | identifier | parenthesized | array_literal | geometry_expr | type_annot ;

literal    = integer | float | string ;
identifier = ident ;
parenthesized = "(" , expression , ")" ;

array_literal = "[" , [ expression , { "," , expression } ] , "]" ;
              | shape_decl , array_literal ;   (* optional leading shape *)

shape_decl = "{⇒" , integer , { "," , integer } , "}" ;  (* shape annotation *)

geometry_expr = "geo" , "(" , manifold , "," , expression , ")" ;

type_annot = expression , ":" , type ;
type       = scalar_type | array_type | refinement ;
```

## Modifiers and higher-order

```ebnf
modifier   = "¨" | "¨¨" | "´" | "´´" | "˙" | "˙˙" | "/¨" | ... ; (* rank, each, fold, scan, etc. *)
modified   = expression , modifier ;
```

## Indexing and structural

```ebnf
index      = expression , "[" , expression , { ";" , expression } , "]" ;
reshape    = expression , "⍡" , expression ;          (* or explicit ⍡ glyph *)
```

## Error behavior (static + runtime)

- Rank error, length error, domain error, index error, type error are first-class and deterministic.
- All failures produce a structured error value with location, kind, and message.
- No silent wrapping or partial results unless explicitly requested via a modifier.

## Operator precedence (highest first)

1. Atoms, parentheses, array literals, indexing
2. Modifiers (¨, /, \, etc.)
3. Monadic application
4. Dyadic application
5. Composition (∘)
6. Trains
7. Binding

This grammar is deliberately closer to BQN's explicit trains and modifiers while retaining APL's rich glyph vocabulary and Klong's compact vector style. Glyph collisions are resolved by reserving a single canonical glyph per primitive (see compatibility table).
