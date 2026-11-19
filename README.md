# HERMETIC ARRAY Ω (HAΩ)

Unified BQN × APL × Klong Geometric Computing Language

## Overview

HAΩ is a self-contained array programming language that unifies the geometric, functional, and symbolic capabilities of BQN, APL, and Klong into one coherent language, one typed IR, one deterministic evaluation model, and one geometric array engine.

## Current Status (iterative loop on master prompt)

- Full master build prompt in `MASTER_BUILD_PROMPT.md`
- **Section II** complete: canonical EBNF grammar (`docs/grammar.md`) and compatibility table (`docs/compatibility.md`)
- **Section III** started: geometric operator inventory skeleton (`docs/operators/README.md`)
- Remaining sections (types, C11 compiler/runtime, Agda/Alloy/TLA+ proofs, hermetic build) follow in subsequent iterations

## Structure

- `docs/` — design documents, grammar, operators
- `src/` — C11 runtime and compiler (next)
- `proofs/` — formal verification (Agda, Alloy, TLA+)
- `tests/` — operator test cases

## Principles

- Exactly one parse per accepted program
- Precise type and shape contracts
- Geometric transformations preferred where valid
- Hermetic, reproducible, deterministic
- Formal obligations documented and discharged where possible

License: research (to be finalized)
