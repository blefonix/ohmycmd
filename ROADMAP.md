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

- [x] Create component skeleton (`CMakeLists.txt`, `src/main.cpp`)
- [x] Add SDK bootstrap and logging
- [x] Implement `OhmyCmd_Register` MVP
- [x] Implement one end-to-end command execution path (`/test` via runtime core)
- [x] Add a tiny Pawn test script for smoke validation

---

## Phase 0 — Foundation (MVP bootstrap)

- [x] **P0 completed**

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

- [x] **P1 runtime MVP completed**
- [x] Runtime core skeleton (registry/parser/dispatcher)

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

- [x] **P2 initial Pawn API**

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
- Utility:
  - `OhmyCmd_Count()`

### [P2] Pawn include

- `ohmycmd.inc` with lightweight helper API (`OhmyCmd_RegisterEx(...)`).

### [P2] Exit Criteria

- [x] Pawn script can register commands on mode init and receive callbacks with params.

---

## Phase 3 — Features for RP servers

- [x] **P3 initial RP feature set**

### [P3] Scope

- [x] Permissions/roles integration hooks.
- [x] Cooldowns (global + per-player).
- [x] Rate limiting / anti-spam.
- [x] Built-in usage/help responses.
- [x] Typed argument parsing helpers (int/float/playerid/string/rest).

### [P3] Deliverables

- [x] Policy system for pre-execution checks.
- [x] Standardized deny reasons/messages (`OMC_DENY_*` + built-in messaging + optional callback override).

### [P3] Exit Criteria

- [x] Production-oriented command flows can be expressed without ad-hoc boilerplate.

---

## Phase 4 — Compatibility & Migration Layer

- [x] **P4 migration layer applied**

### [P4] Scope

- [x] Migration aids for existing command styles (ycmd/zcmd/pawncmd patterns).
- [x] Optional compatibility macros/helpers where safe.
- [x] Migration guide and examples.

### [P4] Deliverables

- [x] `docs/migration.md`
- [x] Side-by-side examples (old style → ohmycmd style)
- [x] Compatibility include (`include/ohmycmd_compat.inc`) + `OhmyCmd_RegisterCompat`

### [P4] Exit Criteria

- [x] Existing gamemode commands can be moved incrementally with minimal risk.

---

## Phase 5 — Hardening & Performance

- [x] **P5 hardening/perf baseline applied**

### [P5] Scope

- [x] Benchmarks: lookup/dispatch latency under heavy command usage.
- [x] Fuzz tests for parser edge cases.
- [x] Memory/lifecycle-oriented regression checks via unit+stress coverage.
- [x] Stress tests with large command sets.

### [P5] Deliverables

- [x] Benchmark tool + published baseline numbers (`benchmarks/dispatch_benchmark.cpp`, `docs/benchmarks.md`).
- [x] Test suite in CI (unit + fuzz + stress via `ctest`).

### [P5] Exit Criteria

- [x] Stable behavior under stress in current CI/local baseline (no crashes in parser/registry stress/fuzz runs).

---

## Phase 6 — Pawn.CMD DX parity (1:1 drop-in) + OMC namespace

- [ ] **P6 in progress (new target): become a true 1:1 DX drop-in alternative to Pawn.CMD**

### [P6] Primary objective

Make existing Pawn.CMD-style scripts work with minimal/no rewrites while standardizing naming to `OMC_*`.

### [P6] Critical TODOs (highest priority)

1. **Namespace transition to `OMC_*` (API-level)**
   - [x] Introduce `OMC_*` native surface as first-class API.
   - [x] Rename current `OhmyCmd_*` API to `OMC_*` in include/docs/examples.
   - [x] Remove transitional `OhmyCmd_*` fallbacks after runtime validation.

2. **Native parity with Pawn.CMD DX (functional 1:1 target)**
   - [x] Add/align full command-management native set (alias/flags/description/getters/rename/exists/delete/array handles/emulate).
   - [x] Ensure array-handle lifecycle and behavior match expected usage patterns.
   - [x] Ensure command/alias metadata operations have equivalent semantics.

3. **Callback flow parity (behavioral 1:1 target)**
   - [ ] Mirror callback lifecycle and call order:
     - `OMC_OnInit` (Pawn.CMD-style init phase)
     - `OnPlayerCommandReceived`
     - command handler execution
     - `OnPlayerCommandPerformed`
   - [ ] Match return-value semantics expected by Pawn.CMD-style scripts (continue/stop/unknown command behavior).

4. **Include/macro DX parity (`Pawn.CMD.inc`-style ergonomics)**
   - [ ] Provide OMC include macros equivalent to:
     - `cmd:`, `CMD`, `COMMAND`
     - `alias:`, `flags:`, `description:`
     - helper-style invocation aliases (`callcmd::...`-like DX)
   - [ ] Preserve filterscript/gamemode auto-init ergonomics comparable to legacy include usage.

5. **Config parity for operator expectations**
   - [ ] Support Pawn.CMD-like config keys/behavior:
     - `CaseInsensitivity`
     - `LegacyOpctSupport`
     - `LocaleName`
     - `UseCaching`
     - `LogAmxErrors`
   - [ ] Document exact compatibility behavior and any intentional deviations.

6. **Drop-in verification suite (must-have gate for P6 exit)**
   - [ ] Add fixture scripts that mirror canonical Pawn.CMD examples (aliases/flags/callback flow/emulation).
   - [ ] Add behavior tests for command + alias arrays, rename/delete, callback ordering, and unknown command path.
   - [ ] Validate both gamemode and filterscript include paths.

### [P6] Exit Criteria

- [ ] A Pawn.CMD-style sample package runs with OMC includes/API and preserves expected behavior.
- [ ] `OMC_*` namespace is primary in docs/examples.
- [ ] Compatibility notes are explicit, small, and tested.

---

## Phase 7 — First Stable Release

### [P7] Target

`v1.0.0`

### [P7] Requirements

- Stable post-parity API surface and include files (`OMC_*` primary).
- Linux build artifacts.
- Installation docs for open.mp (`components/ohmycmd.so`).
- Changelog and semantic versioning policy.
