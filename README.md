# ohmycmd

A modern command processor for open.mp servers.

`ohmycmd` runs as a native open.mp component (`ohmycmd.so`) and gives you a clean, fast command layer with a friendly Pawn include API.

## What you get

- **OMC API** as primary surface (`OMC_*` natives)
- **DX-style command syntax**:
  - `cmd:`, `CMD`, `COMMAND`
  - `alias:`, `flags:`, `description:`
- **Command policy controls**:
  - permission flags
  - global/player cooldowns
  - rate limits
- **Typed argument helpers**:
  - `OMC_ArgInt`, `OMC_ArgFloat`, `OMC_ArgPlayerID`, `OMC_ArgString`, `OMC_ArgRest`
- **Callback pipeline hooks**:
  - `OMC_OnInit`
  - `OnPlayerCommandReceived`
  - `OnPlayerCommandPerformed`
  - `OMC_OnPolicyDeny`
- **Migration helpers** for existing gamemodes (`ohmycmd_compat.inc`)
- **Config file support** for operator-level behavior tuning

---

## Requirements

- open.mp server with component loading enabled
- Linux build/runtime environment
- For many open.mp stacks: **32-bit ELF build** (`ELF32`) is required

---

## Build

```bash
git clone git@github.com:blefonix/ohmycmd.git
cd ohmycmd

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOHMYCMD_BUILD_TESTING=ON
cmake --build build --config Release -j$(nproc)
```

Build output:

- `build/ohmycmd.so`

---

## Install on server

1. Copy component:

```bash
cp build/ohmycmd.so /path/to/server/components/ohmycmd.so
```

2. Copy includes for your Pawn compiler include path:

```bash
cp include/ohmycmd.inc /path/to/qawno/include/
cp include/ohmycmd_compat.inc /path/to/qawno/include/
```

3. Restart server.

---

## Quick start (DX macros)

```pawn
#include <open.mp>
#include <ohmycmd>

flags:ping(0)
alias:ping("p")
description:ping("Simple ping command")
cmd:ping(playerid, const params[])
{
    SendClientMessage(playerid, 0x4AC0E0FF, "pong");
    return 1;
}

public OMC_OnInit()
{
    OMC_SetUsage("ping", "/ping");
    return 1;
}
```

Notes:

- `OMC_Init()` is called automatically by include wrappers on gamemode/filterscript init.
- Use `OMC_RegisterCompat` / `OMC_RegisterCompatEx` for incremental migration scenarios.

---

## Configuration

Default config file location:

- `components/ohmycmd.cfg`

Supported keys:

```ini
CaseInsensitivity = true
LegacyOpctSupport = true
LocaleName = "C"
UseCaching = true
LogAmxErrors = true
```

See full reference: `docs/config.md`

---

## Verification / tests

Run local tests:

```bash
ctest --test-dir build --output-on-failure
```

Drop-in fixture check:

```bash
tests/scripts/verify_dropin.sh
```

Benchmark:

```bash
./build/ohmycmd_bench_dispatch 5000 600000
```

---

## Documentation

- Migration guide: `docs/migration.md`
- Runtime config: `docs/config.md`
- Drop-in verification: `docs/dropin-verification.md`
- Benchmark notes: `docs/benchmarks.md`

---

## License

MIT © 2026-present Nazarii Korniienko
