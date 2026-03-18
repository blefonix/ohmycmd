# ohmycmd

`ohmycmd` is an upcoming command processor plugin for open.mp servers.

## Goals

- Work as an **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable alternative for Pawn command handling

## Status

v0.x. Early development.

Current phase: **P4 (Compatibility & Migration Layer)**.

P4 snapshot includes:

- runtime command registry/parser/dispatcher core,
- policy pre-check system (permissions, cooldowns, rate-limit),
- typed arg helper natives,
- compatibility registration native:
  - `OhmyCmd_RegisterCompat`
- compatibility include:
  - `include/ohmycmd_compat.inc`
  - helpers: `OhmyCmd_MigrateYCMD`, `OhmyCmd_MigrateZCMD`, `OhmyCmd_MigrateLegacy`
- migration docs:
  - `docs/migration.md` with side-by-side examples.

## Build (Linux)

```bash
git clone https://github.com/blefonix/ohmycmd.git
# OR: git clone git@github.com:blefonix/ohmycmd.git
cd ohmycmd
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

Output:

- `build/ohmycmd.so`

Notes:

- Build is configured as 32-bit (`ELF32`) by default for open.mp runtime compatibility.

## Pawn includes (qawno/include)

- `include/ohmycmd.inc`
- `include/ohmycmd_compat.inc`

Tiny smoke script:

- `tests/smoke/ohmycmd_smoke.pwn`
  - registers native + compat commands (`/test`, `/pay`, `/legacy`)
  - demonstrates cooldown/rate-limit setup
  - demonstrates typed args and permission hook

## Migration docs

- See `docs/migration.md`.

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but project modules may have different licenses.
