# ohmycmd

`ohmycmd` is an upcoming command processor plugin for open.mp servers.

## Goals

- Work as an **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable alternative for Pawn command handling

## Status

v0.x. Early development.

Current phase: **P1 (Runtime Core)**.

P1 snapshot includes command registry/parser/dispatcher core, `OhmyCmd_Register` MVP native, and a `/test` end-to-end path validated by smoke script.

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

## P1 Runtime Core Snapshot

Implemented in current branch:

- command parser (`src/command_parser.*`)
- command registry (`src/command_registry.*`)
- dispatcher via `onPlayerCommandText`
- Pawn registration MVP native: `OhmyCmd_Register`

Pawn include (for qawno/include):

- `include/ohmycmd.inc`

Tiny smoke script:

- `tests/smoke/ohmycmd_smoke.pwn`
  - registers `/test` with `OhmyCmd_Register`
  - adds alias `/t`

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but the project modules may have different licenses.
