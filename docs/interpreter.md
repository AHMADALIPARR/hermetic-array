# HAΩ Interpreter (`hao-bqn`)

A C11 interpreter for the repository's BQN-style sources (`lang/*.bqn`),
replacing CBQN. It is built and verified with CompCert (`ccomp`) and links the
HAΩ C11 array engine (`src/array.c`, `src/geom.c`, `src/array_ext.c`,
`src/geom_ext.c`) for the `•hao.*` bridge and the HAΩ glyphs.

These files are additions only. No existing line in the repository was changed.

## Build and test

```sh
make -f Makefile.ext                 # CompCert (default CC=ccomp)
make -f Makefile.ext check           # C tests + BQN tests
make -f Makefile.ext CC=gcc check    # cross-check (also CC=clang)
make -f Makefile.ext check CBQN=/path/to/BQN   # also run plain-BQN tests under CBQN
```

`-D_POSIX_C_SOURCE=200809L` is required because `src/parser.c` calls `strndup`,
which `-std=c11` does not declare. Without it the original `make` fails on GCC 14.

## Usage

```sh
bin-ccomp/hao-bqn lang/engine.bqn -e 'Norm 3‿4'        # HAΩ dialect (default)
bin-ccomp/hao-bqn -b -e '+´ ↕10'                       # strict BQN
bin-ccomp/hao-bqn [-b] [-k] [-e expr | -x expr | file]...
```

Files and `-e` expressions run in order in one session. `-e` prints the value
and `-x` evaluates without printing.

## HAΩ dialect (default)

The rules below are what `lang/*.bqn` needs. They come from how those files are written.

| Form | Meaning |
|------|---------|
| `{ … ⊢ … ⊣ … ⊤ … }` | In blocks without `𝕩`/`𝕨`, `⊢ ⊣ ⊤ ⊥` are positional arguments 1–4 |
| `ℓ` | the whole argument (single argument, or the list of arguments) |
| `Name a b c` | capitalised names and `•names` take every directly following subject: 1 is monadic, 2 is dyadic `w F x`, and blocks take any number |
| `c ? a ◇ b` | conditional (`;` is accepted for `◇`) |
| `\|x\|`, `∣x∣` | magnitude bars |
| `⌊k⌋`, `⌈k⌉` | floor / ceiling brackets; directly followed by a subject they rotate it: `⌊1⌋ v` ≡ `1⌽v` |
| `F˘ x` | `x F x` (so `+´×˘` is the squared norm) |
| `a ⌈ b` | join, inside HAΩ blocks (`s ⌈ cr` in ga.bqn, `x ⌈ Orbit …` in group.bqn) |
| `Name ← ∘` | the name keeps the modifier role (`Compose`) |
| `glyph ← value` | binds a glyph locally (glyph_registry.bqn) |

Precedence follows ordinary mathematics, from loosest to tightest:

1. loose monadic functions (structural, derived, blocks) take the rest of the expression
2. comparisons `< > ≤ ≥ = ≠ ≡ ≢ ⟂` (left associative)
3. other dyadic functions and named infix calls (right associative)
4. `+ - ∨`
5. `× ÷ ∧ | ⊗ ⊙`
6. `⋆ √` (right associative)
7. tight monadic arithmetic (`- ¬ √ …` applied to the next value)
8. named calls

`-b` selects strict BQN: right-to-left evaluation, `˘` is Cells, `⊢ ⊣` are
functions, and there are no bars or brackets.

HAΩ glyphs:

| Glyph | Meaning |
|-------|---------|
| `⍡` | exact reshape (no cycling) |
| `⊗` | outer (tensor) product |
| `⊙` | geometric product of 3-vectors or 8-component multivectors |
| `⟂` | orthogonality test |
| `↻` | rotation |
| `⌁` | geodesic midpoint |
| `⍣"msg"` | raise an error |

`⟕` is undefined, both in BQN and in `glyph_registry.bqn`, so the interpreter
reports `undefined primitive ⟕`.

## Coverage compared with CBQN

Covered:

- numbers (`¯`, `e¯`, `π`, `∞`), characters, strings, `⟨⟩` lists, `‿` strands, `[ ]` arrays
- primitives: `+ - × ÷ ⋆ √ ⌊ ⌈ | ¬ ∧ ∨ < > ≤ ≥ = ≠ ≡ ≢ ⊣ ⊢ ⥊ ∾ ≍ ⋈ ↑ ↓ ↕ ⌽ ⍉ / ⍋ ⍒ ⊏ ⊑ ⊐ ⊒ ∊ ⍷ ⊔ « » !`
- 1-modifiers: `˙ ˜ ˘ ¨ ⌜ ´ ˝ ` ⁼`
- 2-modifiers: `∘ ○ ⊸ ⟜ ⊘ ◶ ⎉ ⍤ ⍟ ⎊ ⌾`
- `⌾` works structurally for selection functions and through `⁼` for invertible ones
- blocks with `𝕩 𝕨 𝕊`, guards `?`/`;`, trains, assignment `← ↩ ⇐`
- system values: `•Show •Out •Repr •Fmt •Type •CurrentError •math.*`, plus `•hao.*`, which bridges to the C engine

Not covered:

- user-defined modifiers (`_m`, `_m_` blocks) and `⚇`
- namespaces and `•Import`, `•file`, `•FLines`
- headers and multiple bodies in blocks; destructuring assignment
- `⁼` only for `+ - × ÷ ⋆ √ ¬ ⊢ ⊣ ⌽ ⍉`; `⍷` and `⍉` only for lists and simple cases
- character arithmetic beyond `+ -`; Unicode is code points only
- the box display form (values print as `2‿3⥊⟨ … ⟩`; `•Repr` matches CBQN's source form)
- performance: values are boxed and there are no SIMD kernels

## Behaviour of the original lang files

The interpreter runs every original `lang/*.bqn` file.
CBQN rejects all of them (`Unknown characters: ℓ ∣ ◇ ⊤ θ η ⍡ ⟕ ⌋ ⊗ ⟂ ⊙ ↻ ⌁ ⍤`).
The tests in `tests/bqn/` record these points:

- `ga.bqn`: `GeoProd` and `Exterior` use `⟕`, so they raise `undefined primitive ⟕`.
  `lang/ga_ext.bqn` (new) adds `Cross`, `GeoProd3` and `Exterior3` in the same style
  without `⟕`. `GeoProd3` agrees with `•hao.GeoProd`.
- `types.bqn`: `Geodesic` writes `Sin (1-t)×θ`, which reads as `(Sin (1-t))×θ`,
  so the midpoint of `1‿0‿0` and `0‿1‿0` is `0.753…`, not `0.7071…`.
  `engine.bqn`'s `Geodesic` and `•hao.Geodesic` give the slerp value.
- `types.bqn`: `Rotate` is elementwise, so a quaternion against a 3-vector gives a length error.
- `sphere.bqn` and `engine.bqn` both define `Evolve`.
  The file loaded last wins: sphere's is a single step, engine's is fuel-bounded.
- `Norm ← +´×˘` is the squared norm (`Norm 3‿4` is `25`).
