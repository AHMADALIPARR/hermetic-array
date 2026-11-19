<p align="center">
  <img src="docs/assets/hermetic-omega-hero.svg" alt="HERMETIC-Ω geometric array-language architecture" width="100%" />
</p>
<h1 align="center">HERMETIC-Ω</h1>
<p align="center">A Unified Recursive Geometric Array Language</p>
<p align="center">BQN · APL · J · Q/K · Klong</p>
<p align="center">Pure Arrays. Geometric Algebra. Deterministic Transformation.</p>

## 1. Research Summary

HERMETIC-Ω (also styled HAΩ) is an experimental research repository exploring a single coherent geometric array calculus that draws from BQN, APL, J, Q/K, and Klong. The computational substrate is array transformations, group actions, and manifold evolution rather than dense matrix materialization where avoidable.

**Current evidence base (as of this commit):**
- C11 array engine with explicit ownership, bounds checks, cache-aligned pool allocator, and 4-wide unrolled pointer-arithmetic kernels (`src/array.c`, `src/geom.c`).
- Pure array-language geometric definitions in BQN, APL, J, Klong, and Q (`lang/`).
- Partial Agda formalizations of shape contracts, sphere exponential map, and unit-norm preservation (`proofs/`).
- Minimal parser and IR skeletons (`src/parser.c`, `src/ir.c`).
- AGPLv3 license.

Nothing in this document claims a complete unified interpreter, full formal verification of all operators, or measured performance numbers. Proposed components are labeled Proposed.

## 2. Language Origins and Design Philosophy

The design inherits:
- From BQN: explicit trains, modifiers, rank polymorphism, structural manipulation.
- From APL: rich primitives, axis-aware operations, inner/outer products, grade.
- From J: tacit composition and rank conjunctions.
- From Q/K: compact vector evaluation and recursive functional style.
- From Klong: lightweight recursion and array-oriented higher-order expressions.

Collisions are resolved by a single canonical meaning per glyph or textual name. See `docs/compatibility.md` and `lang/glyph_registry.bqn`.

The goal is one evaluation model, not five interpreters glued together.

## 3. Mathematical Foundations

Unit sphere:
\[
S^n = \{ x \in \mathbb{R}^{n+1} : \|x\| = 1 \}
\]

Sphere exponential map (implemented in `src/geom.c` and `lang/sphere.bqn`):
\[
\operatorname{Exp}_x(v) = \cos(\|v\|)\, x + \operatorname{sinc}(\|v\|)\, v
\]
with continuous zero case via Taylor expansion of sinc. Preconditions: \(\|x\| = 1\) and \(x \cdot v = 0\).

Special orthogonal group:
\[
\operatorname{SO}(n) = \{ Q \in \mathbb{R}^{n \times n} : Q^T Q = I,\ \det(Q) = 1 \}
\]

Orthogonal invariance:
\[
\|Q x\|^2 = \|x\|^2
\]

Recursive geometric state:
\[
G_{n+1} = \Phi(G_n)
\]
where \(\Phi\) is a pure function preserving declared invariants (unit norm, tangency, shape).

Geometric product (simplified 3D form in `src/geom.c` and `lang/ga.bqn`):
\[
ab = a \cdot b + a \wedge b
\]

## 4. Unified Syntax and Formal Grammar

Canonical EBNF is recorded in `docs/grammar.md`. Every accepted program is required to have exactly one parse. The current parser (`src/parser.c`) is a minimal recursive-descent skeleton for atoms and array literals; full trains, modifiers, and geometry expressions remain partial.

Key grammar features from `docs/grammar.md`:
- Explicit trains and composition.
- Glyphs and identifiers for primitives.
- Array literals and optional shape annotations.
- Deterministic error kinds (rank, length, domain, index, type).

## 5. Module Details: Pure Array Languages (`lang/`)

All files under `lang/` are native source for their respective interpreters. They implement geometric primitives using only the host language’s array operators.

### BQN modules
- `lang/engine.bqn`: Defines `Norm`, `Dot`, `Normalize`, `Sinc`, `SphereExp`, `Geodesic`, `Evolve`, plus structural `Reshape`, `Permute`, `Rotate`. Uses BQN trains and blocks.
- `lang/sphere.bqn`: Focused sphere exponential map with `IsUnit` and `IsTangent` checks. Continuous sinc via Taylor at zero.
- `lang/ga.bqn`: Simplified geometric product and exterior product for 3D (scalar + cross-product component). Reversion and grade extraction.
- `lang/types.bqn`, `lang/group.bqn`, `lang/glyph_registry.bqn`: Type annotations, group actions, and glyph inventory (partial).

### APL modules
- `lang/engine.apl`, `lang/operators.apl`: Sphere exponential map and related operators using APL primitives (`√+×`, circle functions, etc.).
- `lang/group.apl`, `lang/tensor.apl`: Group actions and tensor-level operations.

### J modules
- `lang/engine.j`, `lang/recursion.j`: Tacit-style definitions and recursive evolution.

### Klong modules
- `lang/engine.klong`, `lang/evolution.klong`, `lang/recursion.klong`: Compact recursive definitions of evolution and sphere operations.

### Q/K module
- `lang/q_engine.q`: Vector operations and recursive geometric steps in Q syntax.

These modules are independent. No single process executes them all.

## 6. Module Details: C11 Array Engine (`src/`)

### `src/array.h` / `src/array.c`
- `HaoArray` structure: type, rank, shape, strides, data, element count, ownership, bytes.
- Cache-aligned pool (`HaoPool`, `hao_pool_init` using `aligned_alloc`, 64-byte lines).
- Memory accounting globals: `hao_total_bytes`, `hao_peak_bytes`, `hao_alloc_count`.
- Creation, free, pointer arithmetic offset (`hao_offset`), bounds checks.
- Structural: reshape, permute axes, rotate.
- Metric: inner product (4-wide unrolled), norm, normalize.
- Evolution: `hao_evolve_sphere` with precondition checks (unit norm, tangency) and 4-wide unrolled steps.

### `src/geom.h` / `src/geom.c`
- Low-level: `hao_norm2`, `hao_dot` (pointer-arithmetic / 4-wide), `hao_sinc`, `hao_sphere_exp`.
- Checks: `hao_is_unit`, `hao_is_tangent`, `hao_is_orthogonal`.
- Array-level: `hao_array_sphere_exp`, geometric product, exterior, sandwich (placeholder), group act (identity), geodesic.
- Quaternion multiply and rotate.

### `src/parser.c` / `src/parser.h`
- Minimal recursive-descent parser for atoms, numbers, glyphs, and array literals.
- Node kinds: ATOM, ARRAY, etc.
- Free function for AST.

### `src/ir.c` / `src/ir.h`
- Skeleton IR with ops: LOAD, EXP, DOT, NORM, RESHAPE, PERMUTE, GEO_PROD, EVOLVE.
- `ir_from_node` and `ir_eval` dispatch (partial; sphere exp path wired).

### `src/toolchain.c` / `src/toolchain.h`
- Zero-dependency dispatcher bridging string op names to C kernels (`hao_toolchain_eval`).
- Supports sphere exp, evolve, norm, geoprod, act.
- Hand-rolled main for testing geometric steps on fixed 3-vectors.

### `src/main.c`
- Initializes 1 MiB pool.
- Parses argument, builds IR, evaluates, prints rank/count, reports memory, frees pool.

### `src/glyph.c`
- Glyph handling support (present in tree).

## 7. Module Details: Formal Verification (`proofs/`)

Agda modules provide mathematical sketches, not verified compilation of the C runtime:
- `proofs/Array.agda`: Rank-indexed arrays as `Vec`, shape-indexed element count, bounded indices, lookup total by construction. States that axis permutation preserves element count (reflexive proof).
- `proofs/Sphere.agda`: Norm, dot, sinc, exponential map. States unit-norm preservation under unit and tangent preconditions (reflexive).
- `proofs/Geometry.agda`, `proofs/Operators.agda`, `proofs/Type.agda`: Additional shape and operator contracts.

These are not machine-checked against the C implementation.

## 8. Module Details: Documentation and Tests

- `docs/grammar.md`: Full EBNF for the intended surface syntax.
- `docs/compatibility.md`: Glyph and operator collision resolution across languages.
- `docs/operators/README.md`: Operator inventory skeleton.
- `docs/assets/`: Hero, recursive geometry, and architecture SVGs.
- `docs/ASSET-CREDITS.md`: SVG provenance; no photographs committed.
- `docs/references/bibliography.md`: Background references.
- `tests/test_sphere.c`: C test constructing unit vector and small tangent, calling `hao_array_sphere_exp`, asserting final norm ≈ 1.

## 9. Hermetic Geometric Algebra

Implemented kernels (C11, zero external dependencies):
- Sphere exponential map with unit and tangency checks (`hao_array_sphere_exp`).
- Geodesic interpolation (`hao_geodesic`).
- Simplified geometric product and exterior product.
- Quaternion multiply and rotate.
- Group action (identity placeholder in current code).

Pure counterparts exist in `lang/ga.bqn` and related files.

## 10. Recursive Array Transformation Engine

Evolution operator:
\[
X_{n+1} = \operatorname{Exp}_{X_n}(\eta_n V_n)
\]

C11 implementation: `hao_evolve_sphere` in `src/array.c` (fuel-bounded steps, precondition checks, 4-wide unrolled loops).
Pure implementations: `lang/evolution.klong`, `lang/engine.bqn` Evolve, `lang/q_engine.q`.

![Recursive geometry](docs/assets/recursive-geometry.svg)

The polygons are symbolic state representations only.

## 11. Type, Shape, Rank and Memory Invariants

- Rank- and shape-indexed arrays in the C engine.
- Explicit ownership and free paths.
- Cache-line aligned bump pool (`hao_pool_init`, `aligned_alloc`).
- Bounds checks on every indexed access.
- Memory accounting: current bytes, peak, allocation count, pool usage (`hao_memory_report`).

Agda sketches in `proofs/Array.agda`, `proofs/Type.agda` track shape preservation and refinements. These are not complete machine-checked proofs of the C implementation.

## 12. Geometric Operators and Worked Examples

**Normalization (C11 path):**
```c
HaoArray *nrm = hao_norm(a);
double nv = ((double *)nrm->data)[0];
HaoArray *out = hao_normalize(a);
```

**Sphere exp test (from `tests/test_sphere.c`):**
Unit vector \(x = (1,0,0)\), small tangent \(v = (0,0.1,0)\). Post-condition: final norm ≈ 1 within 1e-8.

**Dot product / inner:**
`hao_inner` uses 4-wide unrolled pointer arithmetic.

Shape errors return NULL or negative codes; they are not silently ignored.

## 13. Compiler Architecture and Language Integration

```
src/
  array.c / array.h     — core array representation, pool, structural ops
  geom.c / geom.h       — metric and geometric kernels
  parser.c / parser.h   — minimal parser
  ir.c / ir.h           — typed IR skeleton
  toolchain.c           — zero-dep dispatcher bridging to pure ops
  main.c                — entry point + memory report
lang/                   — pure BQN/APL/J/Klong/Q definitions
proofs/                 — Agda formalizations
tests/                  — C tests
docs/                   — grammar, compatibility, operators
```

![Architecture](docs/assets/native-array-architecture.svg)

The unified syntax and full elaborator are Proposed. The C11 engine and pure language files exist independently.

## 14. Formal Verification and Property Testing

Agda modules (`proofs/Sphere.agda`, `proofs/Operators.agda`) state unit-norm preservation and shape contracts. They are mathematical sketches, not yet connected to the C runtime by extraction or verified compilation.

C test: `tests/test_sphere.c` checks unit norm after exponential map.

Distinction:
- Mathematical claim: stated in Agda and comments.
- Machine-checked proof: partial / not complete.
- Runtime test: present for sphere exp.
- Unverified: full SO(n) closure, general GA, optimizer correctness.

## 15. Benchmarks and Reproducible Experiments

No benchmark numbers are claimed. Reproducible methodology would record interpreter version, hardware, shape, iteration count, and median. Measurements are not yet available.

## 16. Installation, Usage and Contribution

**Build (C11, zero external dependencies beyond libm):**
```bash
make
./hao "[1 0 0]"
```

Pool is initialized to 1 MiB. Memory report is printed on exit.

**Pure languages:** Run the files in `lang/` with the respective interpreters (BQN, Dyalog/GNU APL, J, Klong, Q).

Contribution: follow AGPLv3. Preserve deterministic failure modes and shape contracts. Do not introduce network dependencies or hidden state in pure operators.

## 17. References, Glossary and Provenance

- Master build prompts are preserved in the repository history and `MASTER_BUILD_PROMPT.md` (when present).
- License: GNU Affero General Public License v3 (`LICENSE`).
- Asset credits: `docs/ASSET-CREDITS.md`.
- No photographic assets are committed.

**Glossary**
- HAΩ / HERMETIC-Ω: the project name.
- Shape contract: element count and rank relations that must hold for an operator.
- Hermetic: deterministic, no undeclared external state or network.
- Proposed: specified but not fully implemented in the current tree.

**Status summary**
- Implemented: array representation, pool, geometric kernels (sphere exp, geodesic, GA stubs), pure language definitions, partial Agda, minimal parser/IR, tests.
- Partial: full grammar, complete type system, full formal proofs.
- Proposed: single unified interpreter, complete optimizer, full SO(n) and Clifford algebra.

This README documents the repository as it exists. It does not invent implementations or results.
