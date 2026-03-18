# ohmycmd

`ohmycmd` is an upcoming command processor plugin for open.mp servers.

## Goals

- Work as an **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable alternative for Pawn command handling

## Status

v0.x. Early development.

Current phase: **P3 (RP-oriented features)**.

P3 snapshot includes:

- runtime command registry/parser/dispatcher core,
- Pawn registration API:
  - `OhmyCmd_Register`
  - `OhmyCmd_AddAlias`
  - `OhmyCmd_SetFlags`
  - `OhmyCmd_SetDescription`
  - `OhmyCmd_SetUsage`
  - `OhmyCmd_SetCooldown`
  - `OhmyCmd_SetRateLimit`
  - `OhmyCmd_Execute`
  - `OhmyCmd_Count`
- policy hooks:
  - `OhmyCmd_OnCheckAccess(playerid, command, flags)`
  - `OhmyCmd_OnPolicyDeny(playerid, command, reason, retry_ms)`
- typed arg helper natives:
  - `OhmyCmd_ArgCount`
  - `OhmyCmd_ArgInt`
  - `OhmyCmd_ArgFloat`
  - `OhmyCmd_ArgPlayerID`
  - `OhmyCmd_ArgString`
  - `OhmyCmd_ArgRest`
- built-in usage/help responses:
  - `/cmd help` or `/cmd ?`
  - usage line shown when handler returns `0`

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
  - registers `/test` and `/pay`
  - demonstrates cooldown/rate-limit setup
  - demonstrates typed args and permission hook

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but project modules may have different licenses.
