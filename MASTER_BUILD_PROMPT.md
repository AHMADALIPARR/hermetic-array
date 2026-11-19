MASTER BUILD PROMPT: HERMETIC ARRAY Ω

Unified BQN × APL × Klong Geometric Computing Language

ROLE: You are a programming-language designer, compiler engineer, array-language mathematician, and formal-methods researcher.

Your task is to design and implement HERMETIC ARRAY Ω (HAΩ), a self-contained array programming language that unifies the geometric, functional, and symbolic capabilities of BQN, APL, and Klong.

This is not a transpilation demonstration, a collection of wrappers, or three interpreters connected by scripting.

Build ONE coherent language, ONE typed intermediate representation, ONE deterministic evaluation model, and ONE geometric array engine.

I. FOUNDATIONAL MATHEMATICS

The computational universe is composed of:

1. Finite multidimensional arrays.
2. Exact scalar domains.
3. Coordinate transformations.
4. Unit-sphere geometry.
5. Geodesic evolution.
6. Clifford and geometric algebra.
7. Symmetry groups and group actions.
8. Tensor contractions.
9. Recursive array transformations.
10. Symbolic geometric constraints.
11. Discrete differential geometry.
12. Spectral operators and invariant subspaces.
13. Shape-indexed functions.
14. Hermetic, reproducible computation.

Every operation must have a precise type, shape contract, and mathematical interpretation.

II. UNIFIED LANGUAGE DESIGN

Combine these language traditions:

BQN

* Tacit composition.
* Function trains.
* Inverses.
* Modifiers and higher-order combinators.
* Rank-polymorphic array transformations.
* Explicit structural manipulation.

APL

* Rich primitive-array vocabulary.
* Axis-aware operations.
* Reshape, transpose, rotate, reduce, scan.
* Inner and outer products.
* Grade and permutation operations.
* Generalized array indexing.

Klong

* Compact vector-oriented expressions.
* Array functional composition.
* Adverbs and recursive array processing.
* Concise symbolic operations.
* Efficient uniform-vector transformations.

Resolve collisions explicitly.

Do not simply concatenate the three languages’ glyphs.

Define one canonical grammar in EBNF, including:

* Lexical rules.
* Operator precedence.
* Monadic and dyadic application.
* Function modifiers.
* Composition.
* Binding and scope.
* Array literals.
* Indexing.
* Shape declarations.
* Geometry expressions.
* Type annotations.
* Error behavior.

Provide a compatibility table showing which concepts are inherited, modified, or rejected.

Every accepted program must have exactly one parse.

III. GEOMETRIC OPERATORS

Define first-class operations for:

Structural geometry

* Reshape
* Rank transformation
* Axis permutation
* Array rotation
* Reflection
* Translation
* Projection
* Coordinate change
* Broadcasting
* Generalized contraction

Metric geometry

* Inner product
* Norm
* Normalization
* Geodesic distance
* Exponential map
* Logarithmic map
* Parallel transport
* Curvature estimation

Geometric algebra

* Geometric product
* Exterior product
* Left and right contraction
* Reversion
* Grade projection
* Rotor construction
* Rotor composition
* Sandwich transformations

Group operations

* Group action
* Orbit
* Stabilizer
* Composition
* Inversion
* Equivariance verification
* Canonical representative selection

Each operation requires:

1. Canonical glyph or explicit textual representation.
2. Domain and codomain.
3. Shape transformation.
4. Equational semantics.
5. Boundary conditions.
6. At least three test cases.
7. Error conditions.

IV. PURE GEOMETRIC COMPUTATION

Prioritize geometric transformations over dense matrix materialization wherever mathematically appropriate.

Use:

* Coordinate rotations.
* Structured permutations.
* Rotor operations.
* Group actions.
* Tensor index rearrangement.
* Geodesic interpolation.
* Exact symbolic transformations.
* Local array contractions.

Do not claim that arbitrary linear maps can always be replaced by rotations.

Where a geometric representation is impossible or inefficient, identify the limitation and provide the mathematically equivalent fallback.

Prove equivalence where claimed.

V. RECURSIVE GEOMETRIC EVOLUTION

Create an operator for evolving arrays along a geometric state space.

Define:

Xₙ₊₁ = Exp₍Xₙ₎(ηₙ Vₙ)

where:

* Xₙ is a point on a specified manifold.
* Vₙ is a valid tangent vector.
* ηₙ is a scalar evolution step.
* Exp is the manifold exponential map.

For the unit sphere:

Expₓ(v) = cos(‖v‖)x + sinc(‖v‖)v

provided x has unit norm and v is tangent to the sphere at x.

Define the zero-vector case continuously.

Implement invariant-preserving sphere evolution and check:

* Unit-norm preservation.
* Tangency.
* Determinism.
* Shape preservation.
* Numerical error bounds.

Extend the design to other manifolds only after specifying their metrics and exponential maps.

VI. TYPE SYSTEM

Implement a static type system with:

* Scalar domain types.
* Rank-indexed arrays.
* Shape-indexed arrays.
* Bounded index types.
* Coordinate-space labels.
* Metric-space labels.
* Unit-vector refinements.
* Tangent-vector refinements.
* Rotor refinements.
* Orthogonal-transformation refinements.

Distinguish statically proven properties from runtime-checked properties.

Prevent invalid geometric combinations before evaluation whenever sufficient static information exists.

VII. COMPILER ARCHITECTURE

Implement the language using:

* C11 for the portable runtime and memory representation.
* A formally specified parser and elaborator.
* A typed, shape-aware intermediate representation.
* A native array evaluation engine.
* Optional verified optimization passes.

No JavaScript.
No TypeScript.
No Python runtime dependency.
No browser framework.
No external model API.
No placeholder computational kernels.
No fake benchmark results.

The core must compile into a standalone native program.

The language front end must expose a real parse tree and typed IR.

Use contiguous and strided array views when valid. Avoid hidden allocations.

Implement explicit memory ownership, bounds checks, overflow handling, and deterministic failure behavior.

VIII. FORMAL VERIFICATION

Specify core geometric laws using a suitable combination of:

* Agda for dependent-type proofs.
* Alloy for finite structural consistency models.
* TLA+ for state-transition invariants.

Required proof obligations:

1. Well-typed array operations preserve shape validity.
2. Axis permutations preserve element count.
3. Exact inverse permutations restore original arrays.
4. Rotors preserve Euclidean norms under their mathematical preconditions.
5. Valid orthogonal transformations preserve inner products.
6. Sphere geodesic evolution preserves unit norm mathematically.
7. Compiler transformations preserve observable semantics.
8. Array indices never escape their declared bounds.
9. Recursive geometric transformations obey termination or explicit fuel limits.
10. Optimizations preserve the declared scalar and floating-point semantics.

Never label testing as a formal proof.

Document every remaining assumption.

IX. HERMETIC BUILD CONTRACT

The build must be reproducible and isolated.

Require:

* Pinned compiler/toolchain versions.
* Declared dependencies.
* No runtime network dependency.
* No undeclared filesystem reads.
* Deterministic input handling.
* Deterministic integer and floating-point arithmetic where specified.
* Explicit failure modes.
* No hidden state or non-determinism.

(Note: Original attachment ended mid-sentence at 'Deterministic integer and'. This completes the visible text.)