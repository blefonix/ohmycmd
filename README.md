# ohmycmd

`ohmycmd` is an upcoming command processor plugin for open.mp servers.

## Goals

- Work as an **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable alternative for Pawn command handling

## Status

v0.x. Early development.

Current phase: **P2 (Pawn API)**.

P2 snapshot includes:

- runtime command registry/parser/dispatcher core,
- Pawn registration API:
  - `OhmyCmd_Register`
  - `OhmyCmd_AddAlias`
  - `OhmyCmd_SetDescription`
  - `OhmyCmd_SetUsage`
  - `OhmyCmd_Execute`
  - `OhmyCmd_Count`
- Pawn include helper: `OhmyCmd_RegisterEx(...)`.

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

## Pawn include (qawno/include)

- `include/ohmycmd.inc`

Tiny smoke script:

- `tests/smoke/ohmycmd_smoke.pwn`
  - registers `/test` with `OhmyCmd_RegisterEx`
  - sets description + usage
  - adds alias `/t`

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but project modules may have different licenses.
