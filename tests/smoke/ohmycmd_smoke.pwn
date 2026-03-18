#include <open.mp>
#include <ohmycmd>
#include <ohmycmd_compat>

#define OMC_FLAG_ADMIN (1)

forward OMC_Test(playerid, const args[]);
forward OMC_Pay(playerid, const args[]);
forward CMD_legacy(playerid, const args[]);
forward OhmyCmd_OnCheckAccess(playerid, const command[], flags);
forward OhmyCmd_OnPolicyDeny(playerid, const command[], reason, retry_ms);

public OnGameModeInit()
{
    if (!OhmyCmd_RegisterEx("test", "OMC_Test", 0, "P3 parser/policy smoke command", "/test <amount> [multiplier] [note]"))
    {
        print("[ohmycmd-smoke] failed to register /test");
        return 1;
    }

    OhmyCmd_AddAlias("test", "t");
    OhmyCmd_SetCooldown("test", 0, 800);
    OhmyCmd_SetRateLimit("test", 3, 5000);

    if (!OhmyCmd_RegisterEx("pay", "OMC_Pay", OMC_FLAG_ADMIN, "Admin-only sample command", "/pay <playerid> <amount>"))
    {
        print("[ohmycmd-smoke] failed to register /pay");
        return 1;
    }

    OhmyCmd_SetCooldown("pay", 250, 1000);

    if (!OhmyCmd_MigrateYCMD("legacy", 0, "Compat migration sample", "/legacy [text]"))
    {
        print("[ohmycmd-smoke] failed to compat-register /legacy");
        return 1;
    }

    printf("[ohmycmd-smoke] registered commands, total=%d", OhmyCmd_Count());
    return 1;
}

public OMC_Test(playerid, const args[])
{
    new amount;
    if (!OhmyCmd_ArgInt(args, 0, amount))
    {
        // Returning 0 lets ohmycmd show built-in usage.
        return 0;
    }

    new Float:multiplier = 1.0;
    OhmyCmd_ArgFloat(args, 1, multiplier);

    new note[96];
    if (!OhmyCmd_ArgRest(args, 2, note, sizeof note))
    {
        format(note, sizeof note, "-");
    }

    new out[160];
    format(out, sizeof out, "[ohmycmd-smoke] /test amount=%d multiplier=%.2f note='%s'", amount, multiplier, note);
    SendClientMessage(playerid, 0x66CCFFFF, out);
    return 1;
}

public OMC_Pay(playerid, const args[])
{
    new targetID, amount;
    if (!OhmyCmd_ArgPlayerID(args, 0, targetID) || !OhmyCmd_ArgInt(args, 1, amount))
    {
        return 0;
    }

    new out[144];
    format(out, sizeof out, "[ohmycmd-smoke] /pay -> target=%d amount=%d", targetID, amount);
    SendClientMessage(playerid, 0x99FF99FF, out);
    return 1;
}

public CMD_legacy(playerid, const args[])
{
    new text[96];
    if (!OhmyCmd_ArgRest(args, 0, text, sizeof text))
    {
        format(text, sizeof text, "(empty)");
    }

    new out[128];
    format(out, sizeof out, "[ohmycmd-smoke] compat /legacy text='%s'", text);
    SendClientMessage(playerid, 0xAAAAFFFF, out);
    return 1;
}

public OhmyCmd_OnCheckAccess(playerid, const command[], flags)
{
    if ((flags & OMC_FLAG_ADMIN) == 0)
    {
        return 1;
    }

    return IsPlayerAdmin(playerid);
}

public OhmyCmd_OnPolicyDeny(playerid, const command[], reason, retry_ms)
{
    if (reason == OMC_DENY_PERMISSION)
    {
        new msg[128];
        format(msg, sizeof msg, "[ohmycmd-smoke] denied: /%s requires admin", command);
        SendClientMessage(playerid, 0xFF6666FF, msg);
        return 1;
    }

    return 0;
}
