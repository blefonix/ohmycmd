#include <open.mp>
#include <ohmycmd>
#include <ohmycmd_compat>

#define OMC_FLAG_ADMIN (1)

forward CMD_legacy(playerid, const args[]);
forward OMC_OnCheckAccess(playerid, const command[], flags);
forward OMC_OnPolicyDeny(playerid, const command[], reason, retry_ms);
forward OMC_OnInit();
forward OnPlayerCommandReceived(playerid, cmd[], params[], flags);
forward OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags);

flags:test(0)
description:test("P6 DX macro command")
alias:test("t")
cmd:test(playerid, const params[])
{
    new amount;
    if (!OMC_ArgInt(params, 0, amount))
    {
        return 0;
    }

    new Float:multiplier = 1.0;
    OMC_ArgFloat(params, 1, multiplier);

    new note[96];
    if (!OMC_ArgRest(params, 2, note, sizeof note))
    {
        format(note, sizeof note, "-");
    }

    new out[160];
    format(out, sizeof out, "[ohmycmd-smoke] /test amount=%d multiplier=%.2f note='%s'", amount, multiplier, note);
    SendClientMessage(playerid, 0x66CCFFFF, out);
    return 1;
}

flags:pay(OMC_FLAG_ADMIN)
description:pay("Admin-only macro command")
alias:pay("p")
cmd:pay(playerid, const params[])
{
    new targetID, amount;
    if (!OMC_ArgPlayerID(params, 0, targetID) || !OMC_ArgInt(params, 1, amount))
    {
        return 0;
    }

    new out[144];
    format(out, sizeof out, "[ohmycmd-smoke] /pay -> target=%d amount=%d", targetID, amount);
    SendClientMessage(playerid, 0x99FF99FF, out);
    return 1;
}

public OMC_OnInit()
{
    OMC_SetUsage("test", "/test <amount> [multiplier] [note]");
    OMC_SetUsage("pay", "/pay <playerid> <amount>");

    OMC_SetCooldown("test", 0, 800);
    OMC_SetRateLimit("test", 3, 5000);
    OMC_SetCooldown("pay", 250, 1000);

    if (!OMC_MigrateYCMD("legacy", 0, "Compat migration sample", "/legacy [text]"))
    {
        print("[ohmycmd-smoke] failed to compat-register /legacy");
        return 1;
    }

    printf("[ohmycmd-smoke] registered commands, total=%d", OMC_Count());
    return 1;
}

public CMD_legacy(playerid, const args[])
{
    new text[96];
    if (!OMC_ArgRest(args, 0, text, sizeof text))
    {
        format(text, sizeof text, "(empty)");
    }

    new out[128];
    format(out, sizeof out, "[ohmycmd-smoke] compat /legacy text='%s'", text);
    SendClientMessage(playerid, 0xAAAAFFFF, out);
    return 1;
}

public OMC_OnCheckAccess(playerid, const command[], flags)
{
    if ((flags & OMC_FLAG_ADMIN) == 0)
    {
        return 1;
    }

    return IsPlayerAdmin(playerid);
}

public OMC_OnPolicyDeny(playerid, const command[], reason, retry_ms)
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

public OnPlayerCommandReceived(playerid, cmd[], params[], flags)
{
    // P6 callback-pipeline smoke hook.
    return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags)
{
    if (result == -1)
    {
        SendClientMessage(playerid, 0xFFFFFFFF, "SERVER: Unknown command.");
        return 1;
    }

    return 0;
}
