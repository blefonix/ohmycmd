#if defined _INC_ohmycmd
    #endinput
#endif
#define _INC_ohmycmd

// Policy deny reasons for OMC_OnPolicyDeny callback.
#define OMC_DENY_PERMISSION       (1)
#define OMC_DENY_GLOBAL_COOLDOWN  (2)
#define OMC_DENY_PLAYER_COOLDOWN  (3)
#define OMC_DENY_RATE_LIMIT       (4)

enum CmdArray
{
    OMC_CMD_ARRAY_INVALID = 0
};

// OMC_* is the primary public API.
native OMC_Register(const name[], const handler[], flags = 0);
native OMC_RegAlias(const cmd[], const alias[], ...);
native OMC_RegisterCompat(const name[], flags = 0);
native OMC_AddAlias(const name[], const alias[]);

native OMC_SetFlags(const name[], flags);
native OMC_GetFlags(const name[]);

native OMC_SetDescription(const name[], const text[]);
native OMC_GetDescription(const name[], output[], size = sizeof output);

native OMC_SetUsage(const name[], const usage[]);
native OMC_SetCooldown(const name[], global_ms = 0, player_ms = 0);
native OMC_SetRateLimit(const name[], max_calls = 0, window_ms = 0);

native OMC_RenameCommand(const name[], const newname[]);
native OMC_CommandExists(const name[]);
native OMC_DeleteCommand(const name[]);

native CmdArray:OMC_GetCommandArray();
native CmdArray:OMC_GetAliasArray(const name[]);
native OMC_GetArraySize(CmdArray:arr);
native OMC_GetCommandName(CmdArray:arr, index, output[], size = sizeof output);
native OMC_FreeArray(&CmdArray:arr);

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
