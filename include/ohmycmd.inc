#if defined _INC_ohmycmd
    #endinput
#endif
#define _INC_ohmycmd

// Policy deny reasons for OhmyCmd_OnPolicyDeny callback.
#define OMC_DENY_PERMISSION       (1)
#define OMC_DENY_GLOBAL_COOLDOWN  (2)
#define OMC_DENY_PLAYER_COOLDOWN  (3)
#define OMC_DENY_RATE_LIMIT       (4)

native OhmyCmd_Register(const name[], const handler[], flags = 0);
native OhmyCmd_RegisterCompat(const name[], flags = 0);
native OhmyCmd_AddAlias(const name[], const alias[]);
native OhmyCmd_SetFlags(const name[], flags);
native OhmyCmd_SetDescription(const name[], const text[]);
native OhmyCmd_SetUsage(const name[], const usage[]);
native OhmyCmd_SetCooldown(const name[], global_ms = 0, player_ms = 0);
native OhmyCmd_SetRateLimit(const name[], max_calls = 0, window_ms = 0);

native OhmyCmd_Execute(playerid, const input[]);
native OhmyCmd_Count();

native OhmyCmd_ArgCount(const input[]);
native bool:OhmyCmd_ArgInt(const input[], index, &value);
native bool:OhmyCmd_ArgFloat(const input[], index, &Float:value);
native bool:OhmyCmd_ArgPlayerID(const input[], index, &playerid);
native bool:OhmyCmd_ArgString(const input[], index, output[], size = sizeof output);
native bool:OhmyCmd_ArgRest(const input[], index, output[], size = sizeof output);

stock bool:OhmyCmd_RegisterEx(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
{
    if (!OhmyCmd_Register(name, handler, flags))
    {
        return false;
    }

    if (description[0] != '\0')
    {
        OhmyCmd_SetDescription(name, description);
    }

    if (usage[0] != '\0')
    {
        OhmyCmd_SetUsage(name, usage);
    }

    return true;
}

stock bool:OhmyCmd_RegisterCompatEx(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    if (!OhmyCmd_RegisterCompat(name, flags))
    {
        return false;
    }

    if (description[0] != '\0')
    {
        OhmyCmd_SetDescription(name, description);
    }

    if (usage[0] != '\0')
    {
        OhmyCmd_SetUsage(name, usage);
    }

    return true;
}
