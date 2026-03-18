# ohmycmd

`ohmycmd` is an upcoming command processor component for open.mp servers.

## Goals

- Work as a native **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable command layer with Pawn.CMD-style migration path

## Status

v0.x. Early development.

Current phase: **P6 completed** (OMC DX parity target).

P6 includes:

- `OMC_*` API namespace as primary (fallback aliases removed),
- callback pipeline (`OMC_OnInit`, `OnPlayerCommandReceived`, handler, `OnPlayerCommandPerformed`),
- DX include macros (`cmd:`, `CMD`, `COMMAND`, `alias:`, `flags:`, `description:`, `callcmd::...`),
- runtime config support (`CaseInsensitivity`, `LegacyOpctSupport`, `LocaleName`, `UseCaching`, `LogAmxErrors`),
- drop-in fixture suite for gamemode + filterscript includes,
- compatibility include helpers for incremental migration,
- P5 baseline assets still active:
  - unit/fuzz/stress tests,
  - dispatch benchmark,
  - CI test + benchmark execution.

## Build (Linux)

```bash
git clone https://github.com/blefonix/ohmycmd.git
# OR: git clone git@github.com:blefonix/ohmycmd.git
cd ohmycmd
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOHMYCMD_BUILD_TESTING=ON
cmake --build build --config Release -j$(nproc)
```

Output:

- `build/ohmycmd.so`

Notes:

- Build is configured as 32-bit (`ELF32`) by default for open.mp runtime compatibility.

## Run tests + benchmark

```bash
ctest --test-dir build --output-on-failure
./build/ohmycmd_bench_dispatch 5000 600000
```

## Pawn includes (qawno/include)

- `include/ohmycmd.inc` (`OMC_*` primary)
- `include/ohmycmd_compat.inc`

Tiny smoke script:

- `tests/smoke/ohmycmd_smoke.pwn`
  - demonstrates macro-based command declaration (`cmd:`, `alias:`, `flags:`, `description:`)
  - demonstrates callback pipeline hooks
  - demonstrates cooldown/rate-limit setup and typed args

## Migration docs

- `docs/migration.md`

## Runtime config docs

- `docs/config.md`

## Drop-in verification docs

- `docs/dropin-verification.md`

## Benchmark baseline

- `docs/benchmarks.md`

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but project modules may have different licenses.
