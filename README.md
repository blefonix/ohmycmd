# ohmycmd

`ohmycmd` is an upcoming command processor component for open.mp servers.

## Goals

- Work as a native **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable command layer with Pawn.CMD-style migration path

## Status

v0.x. Early development.

Current phase: **P6 (Pawn.CMD DX parity + OMC namespace)**.

P6 work in progress currently includes:

- `OMC_*` API namespace introduced as primary,
- temporary legacy alias support for `OhmyCmd_*`,
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

- `include/ohmycmd.inc` (`OMC_*` primary, legacy aliases available)
- `include/ohmycmd_compat.inc`

Tiny smoke script:

- `tests/smoke/ohmycmd_smoke.pwn`
  - registers native + compat commands (`/test`, `/pay`, `/legacy`)
  - demonstrates cooldown/rate-limit setup
  - demonstrates typed args and permission hook

## Migration docs

- `docs/migration.md`

## Benchmark baseline

- `docs/benchmarks.md`

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but project modules may have different licenses.
