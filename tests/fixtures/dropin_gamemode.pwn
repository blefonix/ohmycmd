#include <open.mp>
#include <ohmycmd>

enum (<<= 1)
{
    CMD_ADMIN = 1,
    CMD_MODER,
    CMD_JR_MODER
};

new gPermissions[MAX_PLAYERS];

forward OMC_OnInit();
forward OMC_OnCheckAccess(playerid, const command[], flags);
forward OnPlayerCommandReceived(playerid, cmd[], params[], flags);
forward OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags);

flags:ban(CMD_ADMIN)
alias:ban("block")
description:ban("Ban player")
cmd:ban(playerid, const params[])
{
    return 1;
}

flags:kick(CMD_ADMIN | CMD_MODER)
cmd:kick(playerid, const params[])
{
    return 1;
}

flags:jail(CMD_ADMIN | CMD_MODER | CMD_JR_MODER)
cmd:jail(playerid, const params[])
{
    return 1;
}

public OMC_OnInit()
{
    gPermissions[1] = CMD_ADMIN | CMD_MODER | CMD_JR_MODER;
    gPermissions[2] = CMD_MODER | CMD_JR_MODER;
    gPermissions[3] = CMD_JR_MODER;
    gPermissions[4] = 0;

    OMC_Execute(1, "/ban 4 reason");
    OMC_Execute(2, "/ban 4 reason");
    OMC_Execute(2, "/kick 4 reason");
    OMC_Execute(3, "/jail 4 reason");
    OMC_Execute(4, "/ban 4 reason");

    return 1;
}

public OMC_OnCheckAccess(playerid, const command[], flags)
{
    if ((flags & gPermissions[playerid]) == flags)
    {
        return 1;
    }

    return 0;
}

public OnPlayerCommandReceived(playerid, cmd[], params[], flags)
{
    return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags)
{
    if (result == -1)
    {
        SendClientMessage(playerid, -1, "Unknown command");
        return 1;
    }

    return 0;
}
