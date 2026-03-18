# Migration Guide: ycmd / zcmd / legacy -> ohmycmd

This guide helps migrate command code incrementally without big-bang rewrites.

## 1) ycmd-style (`CMD_<name>`) -> ohmycmd compat

### Before (legacy style)

```pawn
CMD_kick(playerid, params[])
{
    // ...
    return 1;
}
```

### After (minimal migration, keep handler)

```pawn
#include <ohmycmd_compat>

public OnGameModeInit()
{
    OhmyCmd_MigrateYCMD("kick", ADMIN_FLAG, "Kick a player", "/kick <id> [reason]");
    return 1;
}

public CMD_kick(playerid, const args[])
{
    // ...same logic, params -> args
    return 1;
}
```

`OhmyCmd_MigrateYCMD` auto-resolves common public names (`CMD_kick`, `cmd_kick`, etc.) via `OhmyCmd_RegisterCompat`.

---

## 2) zcmd-style (`cmd_<name>`) -> ohmycmd compat

### Before

```pawn
CMD:ban(playerid, params[])
{
    // ...
    return 1;
}
```

### After

```pawn
#include <ohmycmd_compat>

public OnGameModeInit()
{
    OhmyCmd_MigrateZCMD("ban", ADMIN_FLAG, "Ban player", "/ban <id> <hours> [reason]");
    return 1;
}

public cmd_ban(playerid, const args[])
{
    // ...
    return 1;
}
```

---

## 3) Explicit migration for custom handler names

If legacy handlers use custom names, register explicitly:

```pawn
#include <ohmycmd_compat>

public OnGameModeInit()
{
    OhmyCmd_MigrateLegacy("announce", "Legacy_AnnounceHandler", 0, "Broadcast message", "/announce <text>");
    return 1;
}

public Legacy_AnnounceHandler(playerid, const args[])
{
    // ...
    return 1;
}
```

---

## 4) Typed args migration pattern

Replace ad-hoc parsing with helper natives:

```pawn
public CMD_kick(playerid, const args[])
{
    new target;
    if (!OhmyCmd_ArgPlayerID(args, 0, target))
    {
        return 0; // ohmycmd prints usage if configured
    }

    new reason[96];
    OhmyCmd_ArgRest(args, 1, reason, sizeof reason);

    // ...
    return 1;
}
```

---

## 5) Permissions + cooldown + anti-spam

```pawn
public OnGameModeInit()
{
    OhmyCmd_RegisterCompatEx("pay", ADMIN_FLAG, "Admin payment", "/pay <id> <amount>");
    OhmyCmd_SetCooldown("pay", 250, 1000);   // global, per-player
    OhmyCmd_SetRateLimit("pay", 5, 10000);   // max 5 calls / 10s per player
    return 1;
}

public OhmyCmd_OnCheckAccess(playerid, const command[], flags)
{
    if ((flags & ADMIN_FLAG) == 0) return 1;
    return IsPlayerAdmin(playerid);
}
```

---

## Notes

- Prefer incremental migration command-by-command.
- For safe rollout, keep old handlers and swap registration first.
- Return `0` from a handler when you want built-in usage response.
