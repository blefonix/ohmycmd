#if defined _INC_ohmycmd
    #endinput
#endif
#define _INC_ohmycmd

// Policy deny reasons for OMC_OnPolicyDeny callback.
#define OMC_DENY_PERMISSION       (1)
#define OMC_DENY_GLOBAL_COOLDOWN  (2)
#define OMC_DENY_PLAYER_COOLDOWN  (3)
#define OMC_DENY_RATE_LIMIT       (4)

// OMC_* is the primary public API from Phase 6 onward.
native OMC_Register(const name[], const handler[], flags = 0);
native OMC_RegisterCompat(const name[], flags = 0);
native OMC_AddAlias(const name[], const alias[]);
native OMC_SetFlags(const name[], flags);
native OMC_SetDescription(const name[], const text[]);
native OMC_SetUsage(const name[], const usage[]);
native OMC_SetCooldown(const name[], global_ms = 0, player_ms = 0);
native OMC_SetRateLimit(const name[], max_calls = 0, window_ms = 0);

native OMC_Execute(playerid, const input[]);
native OMC_Count();

native OMC_ArgCount(const input[]);
native bool:OMC_ArgInt(const input[], index, &value);
native bool:OMC_ArgFloat(const input[], index, &Float:value);
native bool:OMC_ArgPlayerID(const input[], index, &playerid);
native bool:OMC_ArgString(const input[], index, output[], size = sizeof output);
native bool:OMC_ArgRest(const input[], index, output[], size = sizeof output);

stock bool:OMC_RegisterEx(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
{
    if (!OMC_Register(name, handler, flags))
    {
        return false;
    }

    if (description[0] != '\0')
    {
        OMC_SetDescription(name, description);
    }

    if (usage[0] != '\0')
    {
        OMC_SetUsage(name, usage);
    }

    return true;
}

stock bool:OMC_RegisterCompatEx(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    if (!OMC_RegisterCompat(name, flags))
    {
        return false;
    }

    if (description[0] != '\0')
    {
        OMC_SetDescription(name, description);
    }

    if (usage[0] != '\0')
    {
        OMC_SetUsage(name, usage);
    }

    return true;
}

// Temporary compatibility aliases for legacy OhmyCmd_* scripts.
#if !defined OMC_ENABLE_LEGACY_ALIASES
    #define OMC_ENABLE_LEGACY_ALIASES 1
#endif

#if OMC_ENABLE_LEGACY_ALIASES
    #define OhmyCmd_Register OMC_Register
    #define OhmyCmd_RegisterCompat OMC_RegisterCompat
    #define OhmyCmd_AddAlias OMC_AddAlias
    #define OhmyCmd_SetFlags OMC_SetFlags
    #define OhmyCmd_SetDescription OMC_SetDescription
    #define OhmyCmd_SetUsage OMC_SetUsage
    #define OhmyCmd_SetCooldown OMC_SetCooldown
    #define OhmyCmd_SetRateLimit OMC_SetRateLimit

    #define OhmyCmd_Execute OMC_Execute
    #define OhmyCmd_Count OMC_Count

    #define OhmyCmd_ArgCount OMC_ArgCount
    #define OhmyCmd_ArgInt OMC_ArgInt
    #define OhmyCmd_ArgFloat OMC_ArgFloat
    #define OhmyCmd_ArgPlayerID OMC_ArgPlayerID
    #define OhmyCmd_ArgString OMC_ArgString
    #define OhmyCmd_ArgRest OMC_ArgRest

    #define OhmyCmd_OnCheckAccess OMC_OnCheckAccess
    #define OhmyCmd_OnPolicyDeny OMC_OnPolicyDeny

    stock bool:OhmyCmd_RegisterEx(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
    {
        return OMC_RegisterEx(name, handler, flags, description, usage);
    }

    stock bool:OhmyCmd_RegisterCompatEx(const name[], flags = 0, const description[] = "", const usage[] = "")
    {
        return OMC_RegisterCompatEx(name, flags, description, usage);
    }
#endif
