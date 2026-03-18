#include <open.mp>
#include <ohmycmd>

forward OMC_Test(playerid, const args[]);

public OnGameModeInit()
{
    if (!OhmyCmd_Register("test", "OMC_Test"))
    {
        print("[ohmycmd-smoke] failed to register /test");
        return 1;
    }

    OhmyCmd_AddAlias("test", "t");

    printf("[ohmycmd-smoke] registered /test (/t), total=%d", OhmyCmd_Count());
    return 1;
}

public OMC_Test(playerid, const args[])
{
    new text[144];
    format(text, sizeof(text), "[ohmycmd-smoke] /test OK args='%s'", args);
    SendClientMessage(playerid, 0x66CCFFFF, text);
    return 1;
}
