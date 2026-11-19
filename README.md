# HERMETIC ARRAY Ω (HAΩ)

Unified BQN × APL × Klong Geometric Computing Language

## Overview

HAΩ is a self-contained array programming language that unifies the geometric, functional, and symbolic capabilities of BQN, APL, and Klong.

**Goals:**
- One coherent language
- One typed intermediate representation
- One deterministic evaluation model
- One geometric array engine

## Status

Initial design phase. See MASTER_BUILD_PROMPT.md for the full specification.

## Structure

- `docs/` : Design documents, grammar, operator specs
- `src/` : C11 runtime and compiler
- `proofs/` : Agda, Alloy, TLA+ formalizations
- `tests/` : Test cases for operators

## Build

Pinned toolchain, reproducible, hermetic.

License: TBD (research project)