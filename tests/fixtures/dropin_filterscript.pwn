#define FILTERSCRIPT

#include <open.mp>
#include <ohmycmd>

forward OMC_OnInit();
forward OnPlayerCommandReceived(playerid, cmd[], params[], flags);
forward OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags);

flags:help(0)
alias:help("commands", "cmds")
description:help("Show command help")
cmd:help(playerid, const params[])
{
    new out[96];
    format(out, sizeof out, "help called: %s", params);
    SendClientMessage(playerid, -1, out);
    return 1;
}

public OMC_OnInit()
{
    OMC_SetUsage("help", "/help [topic]");
    return 1;
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
