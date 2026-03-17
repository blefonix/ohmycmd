# ohmycmd Roadmap

A development roadmap for `ohmycmd`: an open.mp command processor implemented as a **native open.mp component** using the **open.mp SDK**.

## Product Goal

Build a modern, stable, and fast command processor for Pawn scripts that:

- works on the latest open.mp versions,
- does not depend on legacy SA:MP plugin tooling,
- has clean developer ergonomics (simple command declaration + help/permissions/cooldowns),
- is production-safe for very large RP gamemodes.

---

## Guiding Principles

- **SDK-first:** use `open.mp-sdk` as the only core runtime dependency.
- **Compatibility-first:** support existing Pawn patterns where practical.
- **Deterministic behavior:** predictable command resolution and error handling.
- **Low overhead:** minimal per-command runtime cost.
- **Clear migration path:** easy switch from ycmd/zcmd/pawncmd patterns.

---

## Phase 0 — Foundation (MVP bootstrap)

### [P0] Scope

- Repository scaffold for component development.
- CMake setup for Linux builds (`ohmycmd.so`).
- SDK integration (`open.mp-sdk`) and minimal CI build job.
- Basic README + ROADMAP + LICENSE (MIT).

### [P0] Deliverables

- `src/main.cpp` with component entry point.
- Component lifecycle hooks wired: ready/reset/free.
- Build output artifact in CI.

### [P0] Exit Criteria

- Component loads in open.mp and logs startup/shutdown cleanly.

---

## Phase 1 — Runtime Core

### [P1] Scope

- Internal command registry (normalized names, aliases, metadata).
- Fast lookup structure (hash map first, optional trie later).
- Command dispatch pipeline:
  1. parse raw text,
  2. resolve command/alias,
  3. run checks,
  4. call Pawn handler,
  5. return status.

### [P1] Deliverables

- Core engine module (`registry`, `dispatcher`, `parser`).
- Structured log output for command execution.

### [P1] Exit Criteria

- `/test` command can be registered and dispatched end-to-end.

---

## Phase 2 — Pawn API (Developer-facing)

### [P2] Scope

Design and implement stable Pawn-facing API.

### [P2] Initial API proposal

- Native registration:
  - `OhmyCmd_Register(const name[], const handler[], flags = 0)`
  - `OhmyCmd_AddAlias(const name[], const alias[])`
- Optional metadata:
  - `OhmyCmd_SetDescription(const name[], const text[])`
  - `OhmyCmd_SetUsage(const name[], const usage[])`
- Optional execution helper:
  - `OhmyCmd_Execute(playerid, const input[])`

### [P2] Pawn include

- `ohmycmd.inc` with lightweight macros for ergonomic declaration.

### [P2] Exit Criteria

- Pawn script can register commands on mode init and receive callbacks with params.

---

## Phase 3 — Features for RP servers

### [P3] Scope

- Permissions/roles integration hooks.
- Cooldowns (global + per-player).
- Rate limiting / anti-spam.
- Built-in usage/help responses.
- Typed argument parsing helpers (int/float/playerid/string/rest).

### [P3] Deliverables

- Policy system for pre-execution checks.
- Standardized error codes/messages.

### [P3] Exit Criteria

- Production-oriented command flows can be expressed without ad-hoc boilerplate.

---

## Phase 4 — Compatibility & Migration Layer

### [P4] Scope

- Migration aids for existing command styles (ycmd/zcmd/pawncmd patterns).
- Optional compatibility macros where safe.
- Migration guide and examples.

### [P4] Deliverables

- `docs/migration.md`
- side-by-side examples (old style → ohmycmd style)

### [P4] Exit Criteria

- Existing gamemode commands can be moved incrementally with minimal risk.

---

## Phase 5 — Hardening & Performance

### [P5] Scope

- Benchmarks: lookup/dispatch latency under heavy command usage.
- Fuzz tests for parser edge cases.
- Memory and lifecycle safety checks.
- Stress tests with large command sets.

### [P5] Deliverables

- benchmark tool + published baseline numbers.
- test suite in CI.

### [P5] Exit Criteria

- Stable behavior under stress, no major leaks/crashes.

---

## Phase 6 — First Stable Release

### [P6] Target

`v1.0.0`

### [P6] Requirements

- Stable API surface and include file.
- Linux build artifacts.
- Installation docs for open.mp (`components/ohmycmd.so`).
- Changelog and semantic versioning policy.
