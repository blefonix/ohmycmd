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

## Design Notes

### AMX interop fundamentals

Use these concepts and APIs for Pawn integration:

- `amx_Register` — register natives
- `amx_FindPublic` + `amx_Exec` — resolve and call Pawn publics
- `amx_GetAddr` / `amx_GetString` / `amx_SetString` — safe parameter and string handling
- `amx_ftoc` / `amx_ctof` — float-cell conversion without bit corruption
- `amx_Allot` / `amx_Push` / `amx_PushArray` / `amx_PushString` / `amx_Release` — explicit memory lifecycle for by-ref args

### Safety rules

- Prefer explicit memory handling and always pair allocation with `amx_Release`.
- Prefer predictable string handling (`amx_GetString`/`amx_SetString`) over shortcut macros.
- Treat callback/public invocation as failure-prone I/O and check return codes.

### Plugin architecture principles

- deterministic command resolution
- strict registration lifecycle
- clear error paths and logs

## Avoid / Not Primary

### Legacy SA:MP-native calling stacks (Invoke-centric flow)

We are not building around Invoke as the core architecture.

### Old SA:MP-specific export/tooling assumptions

- `.def`-driven workflows and old plugin boilerplate are historical references, not primary design constraints.

### ALS-heavy callback-hooking patterns as the core strategy

Useful as migration context, but not the foundation of the new system.

## Architecture Decision

- Runtime model: **open.mp component**
- Core dependency: **open.mp SDK**
- Command engine: own registry/dispatcher/parser design (not PTL/samp-plugin-sdk compatibility layer)

## Practical Implications

1. Build a minimal component lifecycle first (ready/reset/free).
2. Add command registry and dispatch core.
3. Expose Pawn API via natives in `ohmycmd.inc`.
4. Keep migration helpers optional and layered on top.

## Short-term TODOs

- [ ] Create component skeleton (`CMakeLists.txt`, `src/main.cpp`)
- [ ] Add SDK bootstrap and logging
- [ ] Implement `OhmyCmd_Register` MVP
- [ ] Implement one end-to-end command execution path
- [ ] Add a tiny Pawn test script for smoke validation

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
