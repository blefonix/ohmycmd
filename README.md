# ohmycmd

`ohmycmd` is an upcoming command processor plugin for open.mp servers.

## Goals

- Work as an **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable alternative for Pawn command handling

## Status

v0.x. Early development.

Current phase: **P5 (Hardening & Performance)**.

P5 snapshot includes:

- runtime command registry/parser/dispatcher core,
- RP policy features (permissions/cooldowns/rate-limit + typed args),
- compatibility migration layer (`OhmyCmd_RegisterCompat`, `ohmycmd_compat.inc`),
- test suite binaries:
  - parser unit tests,
  - registry unit tests,
  - parser fuzz tests,
  - registry stress tests,
- benchmark binary:
  - `ohmycmd_bench_dispatch`,
- published baseline output in `docs/benchmarks.md`.

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

- `include/ohmycmd.inc`
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
